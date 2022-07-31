.PHONY:serv client

client:client.cc client.hpp ssock.hpp serv.cc servlogin.hpp servfriend.hpp ssock.hpp threadpool.hpp
	g++ -o client client.cc client.hpp ssock.hpp redis.hpp  -lpthread -Wall
	g++ serv.cc servlogin.hpp servfriend.hpp ssock.hpp threadpool.hpp redis.hpp -o serv -lpthread -lhiredis -Wall

.PHONY:clean
clean:
	rm -f serv client