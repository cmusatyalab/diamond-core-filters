#ifndef	_IMG_SEARCH_H_
#define	_IMG_SEARCH_H_	1

#include <gtk/gtk.h>
#include "image_common.h"
#include "common_consts.h"
#include "texture_tools.h"
#include "histo.h"


/* 
 * This file defines all the search classes that
 * are used to keep track of the various search parameters.
 */

typedef enum {
	TESTX_TOK,
	TESTY_TOK,
	STRIDE_TOK,
	SCALE_TOK,
	METRIC_TOK,
	PATCHFILE_TOK,
	MATCHES_TOK,
	METHOD_TOK,
	CHANNEL_TOK,
	NUMF_TOK,
	START_TOK,
	END_TOK,
	MERGE_TOK,
	OVERLAP_TOK,
	SUPPORT_TOK,
} config_types_t;


/*
 * An enumeration of the different seacrch types.
 */
typedef enum {
	TEXTURE_SEARCH,
	RGB_HISTO_SEARCH,
	VJ_FACE_SEARCH,
	OCV_FACE_SEARCH,
	REGEX_SEARCH,
} search_types_t; 


/* forward declaration to we can get a pointer */
class search_set;

class img_search {
public:
	img_search(const char *name, char *descr);
	virtual ~img_search();
	virtual	int 	handle_config(config_types_t conf_type, char *data);
	virtual	void 	edit_search() = 0;
	virtual	void 	write_fspec(FILE* stream) = 0;
	virtual	void 	write_config(FILE* stream, const char *data_dir) = 0;
	virtual	int 	add_patch(RGBImage* img, bbox_t bbox);
	virtual int 	is_example();
	virtual void 	save_edits();
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist) = 0;

	int				is_selected();
	int				is_hl_selected();
	
	void		set_parent(search_set *);
	search_set *	get_parent();

	const char *			get_name() const; 
	int			set_name(const char *new_name);

	void		set_active_val(int val);
	void		set_active_hlval(int val);
	GtkWidget *	img_search_display();
	GtkWidget *	get_search_widget();
	GtkWidget *	get_config_widget();
	GtkWidget *	get_highlight_widget();
	GtkWidget *	get_edit_widget();
	void		close_edit_win();
	/* XXX need to free the above */

    	img_search &operator=(const img_search &rhs);
    	int operator==(const img_search &rhs) const;
    	int operator<(const img_search &rhs) const;

private:
	char *	display_name;
	char *	descript;
	int		search_selected;
	int		hl_selected;
	search_set *	parent_set;
	GtkWidget *	name_entry;
	GtkWidget *	search_label;
	GtkWidget *	adjust_label;
};



/* 
 * The class for doing windowed searches over
 * the image.  This is a subclass of img_search.
 */

class window_search: public img_search {
public:
	window_search(const char *name, char *descr);
	virtual ~window_search();
	virtual	int handle_config(config_types_t conf_type, char *data);
	virtual	void edit_search() = 0;
	virtual	void write_fspec(FILE* stream);
	virtual	void write_config(FILE* stream, const char *data_dir);
	void		save_edits();

	void set_testx(char *data);
	void set_testx(int val);
	int	 get_testx();

	void set_testy(char *data);
	void set_testy(int val);
	int	 get_testy();

	void set_stride(int new_stride);
	void set_stride(char *data);
	int	 get_stride();

	void set_scale(double new_scale);
	void set_scale(char *data);
	double get_scale();

	void set_matches(int new_match);
	void set_matches(char *data);
	int get_matches();

	GtkWidget *	get_window_cntrl();
	void		close_edit_win();

private:
	double		scale;
	GtkObject *	scale_adj;
	int			testx;
	GtkObject *	testx_adj;
	int			testy;
	GtkObject *	testy_adj;
	int			stride;
	GtkObject *	stride_adj;
	int			num_matches;
	GtkObject *	match_adj;
};


typedef struct example_patch {
	char *		file_name;
	char *		source_name;
	int			xoff;
	int			yoff;
	int			xsize;
	int			ysize;
	RGBImage  *	patch_image;
	void	  *	parent;
	TAILQ_ENTRY(example_patch)	link;
} example_patch_t;


typedef	TAILQ_HEAD(example_list_t, example_patch)	example_list_t;

/* 
 * The class for searches using images patches as input.  For
 * now this is a subclass of window_search.  XXX it isn't clear
 * if this is correct, but I don't want to deal with multiple
 * inheritence ...
 */

class example_search: public window_search {
public:
	example_search(const char *name, char *descr);
	virtual ~example_search();
	virtual	int handle_config(config_types_t conf_type, char *data); 
	int add_patch(RGBImage* img, bbox_t bbox);
	int add_patch(char *conf_str);
	void remove_patch(example_patch_t *patch);
	virtual	void edit_search() = 0;
	virtual	void write_fspec(FILE* stream);
	virtual	void write_config(FILE* stream, const char *data_dir);
	virtual int is_example();

	void		save_edits();
	GtkWidget *	example_display();
	void		close_edit_win();

protected:

	example_list_t		ex_plist;	
	GtkWidget *		patch_holder;
	int			num_patches;

private:
	GtkWidget *	build_patch_table();
	void 		update_display();
	GtkWidget *	patch_table;
};

/* 
 * The class for doing windowed searches over
 * the image.  This is a subclass of img_search.
 */
class rgb_histo_search: public example_search {
public:
	rgb_histo_search(const char *name, char *descr);
	~rgb_histo_search(void);

	void 	add_patch();
	void 	remove_patch();
	virtual void 	edit_search();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	/* set the simularity metric, either via string or double */	
	void 		set_simularity(char *data);
	void 		set_simularity(double sim);

	int handle_config(config_types_t conf_type, char *data);
	void rgb_write_state(void);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

	void 		set_bins(int new_bins);
	void		close_edit_win();

private:
	int			bins;
	int			metric;
	histo_type_t		htype;
	GtkObject *	bin_adj;
	double		simularity;
	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	interpolated_box;

};

class texture_search: public example_search {
public:
	texture_search(const char *name, char *descr);
	~texture_search(void);

	void 	add_patch();
	void 	remove_patch();
	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the simularity metric, either via string or double */	
	void 		set_simularity(char *data);
	void 		set_simularity(double sim);

	/* set number of channels via string or double */
	void 		set_channels(char *data);
	void 		set_channels(int num);

	virtual	int	handle_config(config_types_t conf_type, char *data);

	void		set_matches(char *matches);
	void		set_matches(int matches);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int					method;
	double				simularity;
	int					channels;
	texture_dist_t		distance_metric;
	GtkWidget *			distance_menu;

	GtkObject *	sim_adj;
	GtkWidget *	edit_window;
	GtkWidget *	gray_widget;
	GtkWidget *	rgb_widget;
};


class vj_face_search: public window_search {
public:
	vj_face_search(const char *name, char *descr);
	~vj_face_search(void);

	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the min number of faces required */
	void 		set_face_count(char *data);
	void 		set_face_count(int new_count);

	/* set starting detector level */
	void 		set_start_level(char *data);
	void 		set_start_level(int slevel);

	/* set ending detector level */
	void 		set_end_level(char *data);
	void 		set_end_level(int elevel);
	
	void 		update_toggle();


	int handle_config(config_types_t conf_type, char *data);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int			face_count;
	int			start_stage;
	int			end_stage;
	int			do_merge;
	float		overlap_val;

	GtkWidget *	edit_window;
	GtkObject *	count_widget;
	GtkObject *	start_widget;
	GtkObject *	end_widget;
	GtkWidget *	face_merge;
	GtkObject *	merge_overlap;
	GtkWidget *	overlap_widget;
};

class ocv_face_search: public window_search {
public:
	ocv_face_search(const char *name, char *descr);
	~ocv_face_search(void);

	void	save_edits();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);

	virtual void 	edit_search();
	virtual void	close_edit_win();

	/* set the min number of faces required */
	void 		set_face_count(char *data);
	void 		set_face_count(int new_count);

	/* set the min number of faces required */
	void 		set_support(char *data);
	void 		set_support(int new_count);

	int handle_config(config_types_t conf_type, char *data);

	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	int			face_count;
	int			support_matches;

	GtkWidget *	edit_window;
	GtkObject *	count_widget;
	GtkObject *	support_widget;
};


class rgb_img: public img_search {
public:
	rgb_img(const char *name, char *descr);
	~rgb_img(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	int handle_config(config_types_t conf_type, char *data);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
};


class ii_img: public img_search {
public:
	ii_img(const char *name, char *descr);
	~ii_img(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	int handle_config(config_types_t conf_type, char *data);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);


private:
};

class histo_ii: public img_search {
public:
	histo_ii(const char *name, char *descr);
	~histo_ii(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	int handle_config(config_types_t conf_type, char *data);
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
};


class regex_search: public img_search {
public:
	regex_search(const char *name, char *descr);
	~regex_search(void);

	void	save_edits();
	void	edit_search();
	void 	write_fspec(FILE* stream);
	void	write_config(FILE* stream, const char *data_dir);
	int 	handle_config(config_types_t conf_type, char *data);
	void	close_edit_win();
	virtual void 	region_match(RGBImage *img, bbox_list_t *blist);

private:
	char *	search_string;
	GtkWidget	*string_entry;
	GtkWidget	*edit_window;
};


#ifdef __cplusplus
extern "C" {
#endif

/* this needs to be provide by someone calling this library */
void ss_add_dep(img_search *dep);

#ifdef __cplusplus
}
#endif

#endif	/* !_IMG_SEARCH_H_ */
