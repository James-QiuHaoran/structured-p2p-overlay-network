#!/bin/bash

if [ -z $1 ]; then
    echo "Usage: ./deploy.sh NUM_NODES"
    exit 1
fi

echo "Starting server"

docker stack deploy -c docker-compose-server.yml hgfr

export NUM_NODES=$1

IDS=`docker ps -q`
while [ -z $IDS ]; do
    echo "Waiting for server to be ready"
    sleep 5
    IDS=`docker ps -q`    
done
export EVAL_SERVER_IP=`docker inspect --format '{{range .NetworkSettings.Networks}}{{.IPAddress}}{{end}} {{.Name}}' $IDS | grep hgfr_server | cut -d ' ' -f 1`

echo "Starting clients"

docker stack deploy -c docker-compose-client.yml hgfrc
