#!/bin/bash

docker stack deploy -c docker-compose-server.yml hgfr
docker stack deploy -c docker-compose-client.yml hgfrc