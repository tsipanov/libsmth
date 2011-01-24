#ifndef __SMTH_GST_H__
#define __SMTH_GST_H__

#include <gst/gst.h>
#include <gst/base/gstpushsrc.h>

#include <libmms/mmsx.h>

G_BEGIN_DECLS

/* #define's don't like whitespacey bits */
#define GST_TYPE_MMS    (gst_smth_get_type())
#define GST_MMS(obj)    (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_MMS,GstMMS))
#define GST_IS_MMS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_MMS))

#define GST_MMS_CLASS(class) \
	(G_TYPE_CHECK_CLASS_CAST((class),GST_TYPE_MMS,GstMMSClass))
#define GST_IS_MMS_CLASS(class) \
	(G_TYPE_CHECK_CLASS_TYPE((class),GST_TYPE_MMS))

typedef struct
{
  GstPushSrc parent;

  gchar  *uri_name;
  gchar  *current_connection_uri_name;
  guint  connection_speed;
  
  mmsx_t *connection;

} GstMMS;

typedef struct
{
  GstPushSrcClass parent_class;

} GstMMSClass;

GType gst_smth_get_type (void);

G_END_DECLS

static void gst_smth_finalize(GObject * gobject);
static void gst_smth_uri_handler_init(gpointer g_iface, gpointer iface_data);
static gboolean gst_smth_uri_set_uri(GstURIHandler * handler, const gchar * uri);

static void gst_smth_set_property(GObject * object, guint prop_id,
	const GValue * value, GParamSpec * pspec);
static void gst_smth_get_property(GObject * object, guint prop_id,
	GValue * value, GParamSpec * pspec);

static gboolean gst_smth_query (GstBaseSrc * src, GstQuery * query);
static gboolean gst_smth_start (GstBaseSrc * bsrc);
static gboolean gst_smth_stop (GstBaseSrc * bsrc);
static gboolean gst_smth_is_seekable (GstBaseSrc * src);
static gboolean gst_smth_get_size (GstBaseSrc * src, guint64 * size);

static gboolean gst_smth_prepare_seek_segment (GstBaseSrc * src,
	GstEvent * event, GstSegment * segment);
static gboolean gst_smth_do_seek (GstBaseSrc * src, GstSegment * segment);
static GstFlowReturn gst_smth_create (GstPushSrc * psrc, GstBuffer ** buf);

#define GST_LICENSE ""
#define GST_PACKAGE_NAME ""
#define GST_PACKAGE_ORIGIN ""

#endif /* __SMTH_GST_H__ */
