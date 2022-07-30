.PHONY:serv client

client:client.cc client.hpp ssock.hpp serv.cc servlogin.hpp servfriend.hpp ssock.hpp threadpool.hpp
	g++ -o client client.cc client.hpp ssock.hpp  -lpthread
	g++ serv.cc servlogin.hpp servfriend.hpp ssock.hpp threadpool.hpp -o serv -lpthread -lhiredis 

.PHONY:clean
clean:
	rm -f serv client