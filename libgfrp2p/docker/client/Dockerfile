
FROM jitaodocker/hgfr_eval_base

WORKDIR /hgfreval/test

EXPOSE 20202/udp 30303/udp

ENV bootstrap_ip=10.0.0.2

ENTRYPOINT [ "../bin/eval_client", "20202", "30303", bootstrap_ip, "2000" ]
