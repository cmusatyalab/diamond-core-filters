#ifndef _SFIND_SEARCH_H_
#define _SFIND_SEARCH_H_	1


#ifdef __cplusplus
extern "C" {
#endif

/* XXX do we need to make this global ?? */
extern ls_search_handle_t	shandle;

void create_stats_win(ls_search_handle_t shandle, int expert);
void toggle_stats_win(ls_search_handle_t shandle, int expert);
void close_stats_win();
void init_search();

void drain_ring(ring_data_t *ring);
//extern void set_searchlist(int n, const int *gids);

	
#ifdef __cplusplus
}
#endif

#endif	/* ! _SFIND_SEARCH_H_ */

