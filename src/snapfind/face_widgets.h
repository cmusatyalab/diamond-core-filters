
typedef struct fentry_t {
  /* scape list entry */
  char *name;
  char *file;
  fsp_histo_t fsp_info;

  /* associated widgets */
  GtkWidget *cb;		/* activate */
  GtkWidget *slider, *notb;	/* distance value, negate */
  GtkWidget *xstride, *ystride;	/* x and y stride */

  int disabled;
} fentry_t;


#ifdef __cplusplus
extern "C" {
#endif

	
GtkWidget *create_scapes_notebook(GtkWidget *notebook, fentry_t *scapes, int nscapes,
				  GtkWidget **scapes_table);
GtkWidget *create_slider_entry(char *name, float min, float max, GtkWidget **scalep,
			       int show_widgets);

static const GtkAttachOptions FILL = (GtkAttachOptions)(GTK_EXPAND|GTK_SHRINK|GTK_FILL);
static const GtkAttachOptions NOFILL = (GtkAttachOptions)0;


void save_expert_widget(GtkWidget *widget);
void show_expert_widgets();
void hide_expert_widgets();

void add_scape_widgets1(GtkWidget *table, int row, fentry_t *scape);
void add_scape_widgets2(GtkWidget *table, int row, fentry_t *scape);

#ifdef __cplusplus
}
#endif
