
# OSM Data Validation

Tools for validating OSM database integrity


## Check Changeset Timestamps

Reads a changeset dump and a planet file and checks for objects created outside
the time window given in their changeset. Writes out two files, one containing
the data and one the changesets that fail that check.

Usage: `check_changeset_timestamps CHANGESET-INPUT DATA-INPUT CHANGESET-OUTPUT DATA-OUTPUT`


