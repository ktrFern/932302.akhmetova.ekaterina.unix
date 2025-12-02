FROM alpine:latest

RUN apk add --no-cache bash util-linux coreutils

WORKDIR /program

COPY script.sh /program/script.sh
RUN chmod +x /program/script.sh

CMD ["/program/script.sh"]
