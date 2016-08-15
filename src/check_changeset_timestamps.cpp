/*

https://github.com/osmcode/osm-data-validation

Copyright (C) 2016  Jochen Topf <jochen@topf.org>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

*/

#include <cstring>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include <osmium/io/any_compression.hpp>
#include <osmium/io/any_input.hpp>
#include <osmium/io/any_output.hpp>
#include <osmium/io/file.hpp>
#include <osmium/io/input_iterator.hpp>
#include <osmium/osm/changeset.hpp>
#include <osmium/osm/entity_bits.hpp>
#include <osmium/osm/object.hpp>
#include <osmium/util/string.hpp>

int main(int argc, char* argv[]) {
    if (argc != 5) {
        std::cout << "Usage: " << argv[0] << " CHANGESET-INPUT DATA-INPUT CHANGESET-OUTPUT DATA-OUTPUT\n";
        exit(1);
    }

    osmium::io::File changeset_input_file{argv[1]};
    osmium::io::File data_input_file{argv[2]};
    osmium::io::File changeset_error_file{argv[3]};
    osmium::io::File data_error_file{argv[4]};

    std::vector<std::pair<osmium::Timestamp, osmium::Timestamp>> ranges;

    {
        std::cerr << "Reading changesets...\n";
        osmium::io::Reader reader(changeset_input_file, osmium::osm_entity_bits::changeset);
        auto input_range = osmium::io::make_input_iterator_range<osmium::Changeset>(reader);

        for (const osmium::Changeset& changeset : input_range) {
            if (changeset.id() >= ranges.size()) {
                ranges.resize(changeset.id() + 100);
            }
            ranges[changeset.id()] = std::make_pair(changeset.created_at(), changeset.closed_at());
        }

        reader.close();
        std::cerr << "Done.\n";
    }

    std::set<osmium::changeset_id_type> changesets;

    {
        std::cerr << "Reading OSM data...\n";
        osmium::io::Reader reader(data_input_file, osmium::osm_entity_bits::nwr);
        osmium::io::Writer writer(data_error_file);

        while (osmium::memory::Buffer buffer = reader.read()) {
            for (const auto& object : buffer.select<osmium::OSMObject>()) {
                if (object.changeset() < ranges.size()) {
                    auto r = ranges[object.changeset()];
                    if (!(r.first <= object.timestamp() && object.timestamp() <= r.second)) {
                        writer(object);
                        changesets.insert(object.changeset());
                    }
                } else {
                    std::cerr << "Changeset id " << object.changeset() << " not in changeset file. Ignoring it.\n";
                }
            }
        }

        writer.close();
        reader.close();
        std::cerr << "Done.\n";
    }

    ranges.clear();

    {
        std::cerr << "Reading changesets again and writing out errors...\n";
        osmium::io::Reader reader(changeset_input_file, osmium::osm_entity_bits::changeset);
        osmium::io::Writer writer(changeset_error_file);
        while (osmium::memory::Buffer buffer = reader.read()) {
            for (const auto& changeset : buffer.select<osmium::Changeset>()) {
                if (changesets.count(changeset.id())) {
                    writer(changeset);
                }
            }
        }
        writer.close();
        reader.close();
        std::cerr << "Done.\n";
    }

}

