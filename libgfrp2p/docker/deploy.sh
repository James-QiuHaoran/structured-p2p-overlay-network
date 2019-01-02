#!/bin/bash

docker stack deploy -c docker-compose-server.yml hgfr

echo "Waiting for bootstrap to be ready"

sleep 5

export EVAL_SERVER_IP=`docker inspect --format '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}} {{.Name}}' $(docker ps -q) | grep hgfr_server | cut -d ' ' -f 1`

docker stack deploy -c docker-compose-client.yml hgfrc