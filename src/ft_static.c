#include "ft_event.h"

int init_ft_events_in_table(struct trace_event* start, struct trace_event* stop);

/* In the static configuration, the event table symbol is available
 * at linking time.
 */

extern struct trace_event  __start___event_table[];
extern struct trace_event  __stop___event_table[];

int ft_enable_event(unsigned long id)
{
	return ft_enable_event_in_table(id,
					__start___event_table,
					__stop___event_table);
}

int ft_disable_all_events(void)
{
	return ft_disable_all_events_in_table(__start___event_table,
					      __stop___event_table);
}

int ft_disable_event(unsigned long id)
{
	return ft_disable_event_in_table(id,
					 __start___event_table,
					 __stop___event_table);
}

int ft_is_event_enabled(unsigned long id)
{
	return ft_is_event_enabled_in_table(id,
					 __start___event_table,
					 __stop___event_table);
}

int init_ft_events(void)
{
	return init_ft_events_in_table(__start___event_table,
				       __stop___event_table);
}
