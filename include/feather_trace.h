/*
 * Copyright (c) 2007, Bjoern B. Brandenburg
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the University of North Carolina nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY Bjoern B. Brandenburg ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL Bjoern B. Brandenburg BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _FEATHER_TRACE_H_
#define _FEATHER_TRACE_H_

#define feather_callback __attribute__((regparm(0)))

/* make the compiler reload any register that is not saved in 
 * a cdecl function call
 */
#define __FT_CLOBBER 	"memory", "cc", "eax", "ecx", "edx"
#define __FT_PAR 	"ri"

#define ft_event(id, callback)                                  \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
	    " call " #callback "                          \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : : : __FT_CLOBBER)                            

#define ft_event0(id, callback)                                 \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
            " pushl $" #id  "                             \n\t" \
	    " call " #callback "                          \n\t" \
	    " addl $4, %%esp                              \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : :  : __FT_CLOBBER)                            

#define ft_event1(id, callback, param)                          \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
	    " pushl %0                                    \n\t" \
            " pushl $" #id  "                             \n\t" \
	    " call " #callback "                          \n\t" \
	    " addl $8, %%esp                              \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : : __FT_PAR (param)  : __FT_CLOBBER)                            

#define ft_event2(id, callback, param, param2)                  \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
	    " pushl %1                                    \n\t" \
	    " pushl %0                                    \n\t" \
            " pushl $" #id  "                             \n\t" \
	    " call " #callback "                          \n\t" \
	    " addl $12, %%esp                             \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : : __FT_PAR (param), __FT_PAR (param2)  : __FT_CLOBBER)


#define ft_event3(id, callback, p, p2, p3)                      \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
	    " pushl %2                                    \n\t" \
	    " pushl %1                                    \n\t" \
	    " pushl %0                                    \n\t" \
            " pushl $" #id  "                             \n\t" \
	    " call " #callback "                          \n\t" \
	    " addl $16, %%esp                             \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : : __FT_PAR (p), __FT_PAR (p2), __FT_PAR (p3)  : __FT_CLOBBER)

#define ft_event4(id, callback, p, p2, p3, p4)                  \
        __asm__ __volatile__(                                   \
            "1: jmp 2f                                    \n\t" \
	    " pushl %3                                    \n\t" \
	    " pushl %2                                    \n\t" \
	    " pushl %1                                    \n\t" \
	    " pushl %0                                    \n\t" \
            " pushl $" #id  "                             \n\t" \
	    " call " #callback "                          \n\t" \
	    " addl $20, %%esp                             \n\t" \
            ".section __event_table, \"aw\"               \n\t" \
            ".long " #id  ", 0, 1b, 2f                    \n\t" \
            ".previous                                    \n\t" \
            "2:                                           \n\t" \
        : : __FT_PAR (p), __FT_PAR (p2), __FT_PAR (p3),         \
	    __FT_PAR (p4)  : __FT_CLOBBER)


static inline unsigned long long ft_read_tsc(void) 
{
	unsigned long long ret;
	__asm__ __volatile__("rdtsc" : "=A" (ret));
	return ret;
}

int ft_enable_event(unsigned long id);
int ft_disable_event(unsigned long id);
int ft_is_event_enabled(unsigned long id);
int ft_disable_all_events(void);
#endif
