FROM bdps_run:1.0 

WORKDIR /ppp/serv/bin

ENV SOFTNAME bdpsdatran

COPY ./bin/$SOFTNAME ./$SOFTNAME

RUN chmod 755 ./$SOFTNAME

EXPOSE 2102

CMD exec ./$SOFTNAME
