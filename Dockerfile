FROM bdps_run:1.0 

WORKDIR /ppp/serv/bin

COPY ./bin/bdpsdatran ./bdpsdatran
RUN chmod 755 ./bdpsdatran

EXPOSE 2102

CMD exec ./bdpsdatran 
