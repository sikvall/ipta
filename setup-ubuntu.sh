#!/bin/bash

# Setting up the dev environemnt on a standard Ubuntu box
echo
echo "  **************************************************************"
echo "  This script should install everything needed to build ipta"
echo "  on a standard Ubuntu box. If you already have the packages"
echo "  installed it will instead upgrade you to the latest version"
echo "  should there be a newer one available."
echo "  **************************************************************"
echo "  YOU NEED TO RUN THIS AS ROOT"
echo "  That means likely sudo $0"
echo "  **************************************************************"

read -n 1 -s -p "    Press almost any key to continue or CTRL-C to abort."

apt-get install mysql-common mysql-client mysql-server \
	libmysqlclient-dev build-essential texlive-full git
