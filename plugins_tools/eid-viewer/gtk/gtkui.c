#include <config.h>
#include "gtkui.h"
#include "gettext.h"
#include "gtk_globals.h"
#include "oslayer.h"

#include <locale.h>
#include <stdlib.h>

#ifdef _
#error _ must not be defined
#endif
#define _(s) gettext(s)

#define GEN_FUNC(n, d) \
void n(GtkMenuItem* item, gpointer user_data) { \
	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "mainwin")); \
	GtkWidget* dlg = gtk_message_dialog_new(window, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, d " (not yet implemented)", user_data); \
	gtk_dialog_run(GTK_DIALOG(dlg)); \
	gtk_widget_destroy(dlg); \
}

void showabout(GtkMenuItem* about, gpointer user_data G_GNUC_UNUSED) {
	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "mainwin"));
	gtk_show_about_dialog(window, "program-name", _("eID Viewer"), NULL);
}

static void update_preview(GtkFileChooser* chooser, gpointer data) {
	GtkWidget* preview;
	char *filename;
	GdkPixbuf *pixbuf;
	GInputStream* mstream;
	struct eid_vwr_preview* prv;

	preview = GTK_WIDGET(data);
	filename = gtk_file_chooser_get_preview_filename(chooser);
	prv = eid_vwr_get_preview(filename);
	g_free(filename);

	if(!prv->have_data) {
		gtk_file_chooser_set_preview_widget_active(chooser, FALSE);
		free(prv);
		return;
	}

	mstream = G_INPUT_STREAM(g_memory_input_stream_new_from_data(prv->imagedata, prv->imagelen, NULL));
	pixbuf = gdk_pixbuf_new_from_stream(mstream, NULL, NULL);
	gtk_image_set_from_pixbuf(GTK_IMAGE(preview), pixbuf);
	g_object_unref(pixbuf);
	free(prv->imagedata);
	free(prv);
	gtk_file_chooser_set_preview_widget_active(chooser, TRUE);
}

GEN_FUNC(open_file_detail, "opening %s")

void file_open(GtkMenuItem* item, gpointer user_data) {
	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "mainwin"));
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
			_("Open eID file"), window, GTK_FILE_CHOOSER_ACTION_OPEN,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Open"), GTK_RESPONSE_ACCEPT,
			NULL);
	GtkWidget* preview;
	gchar* filename;
	gint res;
	GtkFileFilter* filter;

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.eid");
	gtk_file_filter_add_pattern(filter, "*.csv");
	gtk_file_filter_set_name(filter, _("All eID files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.eid");
	gtk_file_filter_set_name(filter, _("XML eID files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*.csv");
	gtk_file_filter_set_name(filter, _("CSV eID files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	preview = gtk_image_new();
	gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), preview);
	g_signal_connect(G_OBJECT(dialog), "update-preview", G_CALLBACK(update_preview), preview);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		open_file_detail(NULL, filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

GEN_FUNC(save_file_detail, "saving %s")

void file_save(GtkMenuItem* item, gpointer user_data) {
	GtkWindow* window = GTK_WINDOW(gtk_builder_get_object(builder, "mainwin"));
	GtkWidget* dialog = gtk_file_chooser_dialog_new(
			strcmp(user_data, "xml") ? _("Save CSV eID file") : _("Save XML eID file"), window, GTK_FILE_CHOOSER_ACTION_SAVE,
			_("_Cancel"), GTK_RESPONSE_CANCEL,
			_("_Save"), GTK_RESPONSE_ACCEPT,
			NULL);
	gchar* filename;
	gint res;
	GtkFileFilter* filter;

	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, "*");
	gtk_file_filter_set_name(filter, _("All files"));
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	filter = gtk_file_filter_new();
	if(strcmp(user_data, "xml")) {
		gtk_file_filter_add_pattern(filter, "*.csv");
		gtk_file_filter_set_name(filter, _("CSV eID files"));
	} else {
		gtk_file_filter_add_pattern(filter, "*.eid");
		gtk_file_filter_set_name(filter, _("XML eID files"));
	}
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_filter(GTK_FILE_CHOOSER(dialog), filter);

	res = gtk_dialog_run(GTK_DIALOG(dialog));
	if(res == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		save_file_detail(NULL, filename);
		g_free(filename);
	}
	gtk_widget_destroy(dialog);
}

void file_close(GtkMenuItem* item, gpointer user_data) {
	eid_vwr_deserialize(NULL, 0, NULL);
}

void pinop(GtkWidget* button, gpointer which) {
	enum eid_vwr_pinops op = (enum eid_vwr_pinops) which;

	eid_vwr_pinop(op);
}

void translate(GtkMenuItem* item, gpointer target) {
	enum eid_vwr_langs lang = EID_VWR_LANG_EN;
	GtkWidget *widget;
	char* newlocale;
	if(!strncmp(target, "de", 2)) {
		lang = EID_VWR_LANG_DE;
	} else if(!strncmp(target, "fr", 2)) {
		lang = EID_VWR_LANG_FR;
	} else if(!strncmp(target, "nl", 2)) {
		lang = EID_VWR_LANG_NL;
	}
	convert_set_lang(lang);
	setlocale(LC_MESSAGES, target);

	// TODO: it would be more efficient if the below was parsed from the
	// Glad XML data rather than be re-encoded here...
#define TSL_LABEL(l, t) { widget = GTK_WIDGET(gtk_builder_get_object(builder, l)); gtk_label_set_text(GTK_LABEL(widget), t); }
	TSL_LABEL("tab_id", _("Identity"));
	TSL_LABEL("tab_cardpin", _("Card/PIN"));
	TSL_LABEL("tab_certs", _("Certificates"));
	TSL_LABEL("nametit", _("Name:"));
	TSL_LABEL("giventit", _("Given names:"));
	TSL_LABEL("pobtit", _("Place of birth:"));
	TSL_LABEL("dobtit", _("Date of birth:"));
	TSL_LABEL("sextit", _("Sex:"));
	TSL_LABEL("natnumtit", _("National number:"));
	TSL_LABEL("nattit", _("Nationality:"));
	TSL_LABEL("tittit", _("Title:"));
	TSL_LABEL("spectit", _("Special status:"));
	TSL_LABEL("streettit", _("Street:"));
	TSL_LABEL("posttit", _("Postal code:"));
	TSL_LABEL("munictit", _("Municipality:"));
	TSL_LABEL("cardnumtit", _("Card number:"));
	TSL_LABEL("issuetit", _("Place of issue:"));
	TSL_LABEL("chipnumtit", _("Chip number:"));
	TSL_LABEL("cardvalfromtit", _("Valid from:"));
	TSL_LABEL("cardvaltiltit", _("Valid until:"));
	TSL_LABEL("certvalfromtit", _("Valid from:"));
	TSL_LABEL("certvaltiltit", _("Valid until:"));
	TSL_LABEL("certusetit", _("Use:"));
	TSL_LABEL("certtrusttit", _("Trust:"));
#undef TSL_LABEL
}

GEN_FUNC(file_prefs, "set preferences")
GEN_FUNC(file_print, "print")
GEN_FUNC(showurl, "show %s url")
GEN_FUNC(showlog, "show log tab")
