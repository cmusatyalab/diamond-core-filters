/*
 *  SnapFind
 *  An interactive image search application
 *  Version 1
 *
 *  Copyright (c) 2002-2005 Intel Corporation
 *  All Rights Reserved.
 *
 *  This software is distributed under the terms of the Eclipse Public
 *  License, Version 1.0 which can be found in the file named LICENSE.
 *  ANY USE, REPRODUCTION OR DISTRIBUTION OF THIS SOFTWARE CONSTITUTES
 *  RECIPIENT'S ACCEPTANCE OF THIS AGREEMENT
 */

#ifndef _SFIND_SEARCH_H_
#define _SFIND_SEARCH_H_	1


#ifdef __cplusplus
extern "C"
{
#endif

/* XXX do we need to make this global ?? */
extern ls_search_handle_t	shandle;

void create_stats_win(ls_search_handle_t shandle, int expert);
void toggle_stats_win(ls_search_handle_t shandle, int expert);
void close_stats_win();

void create_progress_win(ls_search_handle_t shandle, int expert);
void toggle_progress_win(ls_search_handle_t shandle, int expert);
void close_progress_win();

void create_ccontrol_win(ls_search_handle_t shandle, int expert);
void toggle_ccontrol_win(ls_search_handle_t shandle, int expert);
void close_ccontrol_win();

void init_search();

void drain_ring(ring_data_t *ring);

#ifdef __cplusplus
}
#endif

#endif	/* ! _SFIND_SEARCH_H_ */

