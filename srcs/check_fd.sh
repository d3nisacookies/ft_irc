#!/bin/bash

watch -n1 "lsof -p $(pidof ircserv)"
