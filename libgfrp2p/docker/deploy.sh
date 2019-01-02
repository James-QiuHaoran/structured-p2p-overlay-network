#!/bin/bash

if [ -z $1 ]; then
    echo "Usage: ./deploy.sh NUM_NODES"
    exit 1
fi

echo "Starting server"

docker stack deploy -c docker-compose-server.yml hgfr

export NUM_NODES=$1

export EVAL_SERVER_IP=`docker inspect --format '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}} {{.Name}}' $(docker ps -q) | grep hgfr_server | cut -d ' ' -f 1`
while [ -z $EVAL_SERVER_IP ]; do
    echo "Waiting for server to be ready"
    sleep 5
    export EVAL_SERVER_IP=`docker inspect --format '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}} {{.Name}}' $(docker ps -q) | grep hgfr_server | cut -d ' ' -f 1`
done

echo "Starting clients"

docker stack deploy -c docker-compose-client.yml hgfrc