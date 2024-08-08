vmcache: *.cpp tpcc/*pp helpers/*pp buffer_manager/*pp
	g++ -DNDEBUG -O3 -std=c++20 -g -fnon-call-exceptions -fasynchronous-unwind-tables vmcache.cpp -o vmcache -laio

clean:
	rm vmcache
