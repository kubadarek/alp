#include "reactor.h"
#include "user_list.h"
#include <sys/epoll.h>
#include <string.h>
#include <unistd.h>

#define MAX_IDX (MAX_USERS-1)
// current_idx to będzie maksymalna liczba event handlerów obecnych w systemie, przechowuje nasz jedyny niepowtarzalny epoll_fd
typedef struct reactor_core {
	int epoll_fd;
	size_t current_idx;
	event_handler* ehs[MAX_IDX];
} reactor_core;
// ma znależź konkretny event_handler, event_handler ma się opiekować zdarzeniem
static event_handler* find_eh(reactor_core* rc, int fd, size_t* idx)
{
	size_t i = 0;
	event_handler* eh = 0;
	for (i = 0; i <= rc->current_idx; i++) {
		if (rc->ehs[i] && (rc->ehs[i]->fd == fd)) {
			eh = rc->ehs[i];
			if (idx)
				*idx = i;
			break;
		}
	}

	return eh;
}
// argument eh jest w mainie jako acceptor, epollfd mieści  w sobie wszystkie event-handlery na konkretnych deskryptorach?
static void add_eh(reactor* self, event_handler* eh)
{
	struct epoll_event ee;
	memset(&ee, 0, sizeof(ee));
	ee.events = EPOLLIN;  //udzielamy prawa odczytu do ee.events, która ma opisywać monitorowane zdarzenia na deskryptorze
	ee.data.fd = eh->fd;  
	epoll_ctl(self->rc->epoll_fd, EPOLL_CTL_ADD, eh->fd, &ee);

	if (self->rc->current_idx < MAX_IDX) 
	{
		if ((self->rc->current_idx == 0) && (self->rc->ehs[0] == 0))
			self->rc->ehs[0] = eh;
		else
			self->rc->ehs[++(self->rc->current_idx)] = eh;
	}
}

static void rm_eh(reactor* self, int fd)
{
	size_t i = 0;
	event_handler* eh = find_eh(self->rc, fd, &i);
	if (!eh)
		return;

	if (i < self->rc->current_idx)
		self->rc->ehs[i] = self->rc->ehs[self->rc->current_idx];

	self->rc->ehs[self->rc->current_idx] = 0;
	if (self->rc->current_idx > 0) {
		--(self->rc->current_idx);
	}

	epoll_ctl(self->rc->epoll_fd, EPOLL_CTL_DEL, eh->fd, 0);
	close(eh->fd);
	free(eh);
}

static void event_loop(reactor* self)
{
	int i = 0;
	int epoll_fd = self->rc->epoll_fd;
	struct epoll_event es[MAX_USERS];
	event_handler* eh = 0;

	for(;;) 
	{
		i = epoll_wait(epoll_fd, es, MAX_USERS, -1);
		for (--i; i > -1; --i) 
		{
			eh = find_eh(self->rc, es[i].data.fd, 0);
			if (eh)
				eh->handle_event(eh, es[i].events);
		}
	}
}

reactor* create_reactor(int epoll_fd)
{
	reactor* r = malloc(sizeof(reactor));
	r->rc = malloc(sizeof(reactor_core));
	r->rc->ehs[0] = 0;

	r->rc->epoll_fd = epoll_fd;
	r->rc->current_idx = 0;
	r->add_eh = &add_eh;
	r->rm_eh = &rm_eh;
	r->event_loop = &event_loop;

	return r;
}

void destroy_reactor(reactor* r)
{
	free(r->rc);
	free(r);
}

