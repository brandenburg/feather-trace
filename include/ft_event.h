#ifndef _FEATHER_TRACE_EVENTS_H_
#define _FEATHER_TRACE_EVENTS_H_

/* for each event, there is an entry in the event table */
struct trace_event {
	unsigned long	id;
	long		count;
	unsigned long	start_addr;
	unsigned long	end_addr;
};

int ft_is_event_enabled_in_table(unsigned long id,
				 struct trace_event* te, /* start of table */
				 struct trace_event* stop);

int ft_disable_all_events_in_table(struct trace_event* te, /* start of table */
				   struct trace_event* stop);

int ft_disable_event_in_table(unsigned long id,
			      struct trace_event* te, /* start of table */
			      struct trace_event* stop);

int ft_enable_event_in_table(unsigned long id,
			     struct trace_event* te, /* start of table */
			     struct trace_event* stop);

#endif
