#include "config.h" // IWYU pragma: export

#include "note-preferences.h"

struct _NotePreferences {
  AdwApplicationWindow parent_instance;

  /* Template widgets */
  GSettings *settings;
  AdwPreferencesPage *apperence;
  AdwPreferencesPage *editor;
  AdwPreferencesPage *sync;
  AdwPreferencesPage *save;
  GtkAdjustment *title_num;
  GtkAdjustment *subtitle_num;
  GtkAdjustment *subtitle_line;
  GtkSwitch *custom_font;
  GtkFontDialogButton *custom_font_border;
  GtkFontDialogButton *custom_font_title;
  GtkFontDialogButton *custom_font_subtitle;
  GtkSwitch *custom_window_size;
  GtkAdjustment *height;
  GtkAdjustment *width;
  GtkSwitch *custom_editor_font;
  GtkFontDialogButton *custom_font_editor;
  AdwEntryRow *save_path;
  GtkAdjustment *lock_time;
  GtkButton *file_choose;
  GtkButton *webdav_test;
  AdwEntryRow *webdav_account;
  AdwPasswordEntryRow *webdav_password;
  AdwEntryRow *webdav_addr;
  GtkLabel *webdav_result;
};

G_DEFINE_FINAL_TYPE(NotePreferences, note_preferences,
                    ADW_TYPE_PREFERENCES_WINDOW)

static void note_preferences_class_init(NotePreferencesClass *klass) {
  GtkWidgetClass *widget_class = GTK_WIDGET_CLASS(klass);

  gtk_widget_class_set_template_from_resource(
      widget_class, "/org/lion_claw/note/note-preferences.ui");
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       apperence);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences, editor);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences, sync);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences, save);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       title_num);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       subtitle_num);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       subtitle_line);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_font);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_font_border);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_font_title);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_font_subtitle);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_window_size);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences, height);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences, width);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_editor_font);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       custom_font_editor);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       save_path);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       lock_time);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       file_choose);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       webdav_test);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       webdav_account);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       webdav_password);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       webdav_addr);
  gtk_widget_class_bind_template_child(widget_class, NotePreferences,
                                       webdav_result);
}

static void note_preferences_close(NotePreferences *self) {
  g_settings_set_uint(self->settings, "height",
                      (uint)gtk_adjustment_get_value(self->height));
  g_settings_set_uint(self->settings, "width",
                      (uint)gtk_adjustment_get_value(self->width));
  g_settings_set_boolean(self->settings, "custom-window-size",
                         gtk_switch_get_active(self->custom_window_size));
  g_settings_set_uint(self->settings, "title-num",
                      (uint)gtk_adjustment_get_value(self->title_num));
  g_settings_set_uint(self->settings, "subtitle-num",
                      (uint)gtk_adjustment_get_value(self->subtitle_num));
  g_settings_set_uint(self->settings, "subtitle-line",
                      (uint)gtk_adjustment_get_value(self->subtitle_line));
  g_settings_set_boolean(self->settings, "custom-font",
                         gtk_switch_get_active(self->custom_font));
  g_settings_set_boolean(self->settings, "custom-editor-font",
                         gtk_switch_get_active(self->custom_editor_font));
  g_settings_set_string(
      self->settings, "custom-font-border",
      pango_font_description_to_string(
          gtk_font_dialog_button_get_font_desc(self->custom_font_border)));
  g_settings_set_string(
      self->settings, "custom-font-title",
      pango_font_description_to_string(
          gtk_font_dialog_button_get_font_desc(self->custom_font_title)));
  g_settings_set_string(
      self->settings, "custom-font-subtitle",
      pango_font_description_to_string(
          gtk_font_dialog_button_get_font_desc(self->custom_font_subtitle)));
  g_settings_set_string(
      self->settings, "custom-font-editor",
      pango_font_description_to_string(
          gtk_font_dialog_button_get_font_desc(self->custom_font_editor)));
  g_settings_set_int(self->settings, "lock-time",
                     (int)gtk_adjustment_get_value(self->lock_time));
  g_settings_set_string(self->settings, "save-path",
                        gtk_editable_get_text(GTK_EDITABLE(self->save_path)));
  g_settings_set_string(
      self->settings, "webdav-account",
      gtk_editable_get_text(GTK_EDITABLE(self->webdav_account)));
  g_settings_set_string(self->settings, "webdav-addr",
                        gtk_editable_get_text(GTK_EDITABLE(self->webdav_addr)));
  // g_settings_set_string(
  //     self->settings, "webdav-passwd",
  //     gtk_editable_get_text(GTK_EDITABLE(self->webdav_password)));
  // TODO - fontdiagbutton
  // 因为只能显示字体和大小，所以即便fontdiag能够设置weight和language之类的信息也不会到fontdiagbutton,只能说适合高级设置。
  gtk_window_close(GTK_WINDOW(self));
}

static void open_finish(GObject *dialog, GAsyncResult *res,
                        gpointer user_data) {
  GFile *file = gtk_file_dialog_open_finish(GTK_FILE_DIALOG(dialog), res, NULL);
  NotePreferences *self = NOTE_PREFERENCES(user_data);
  char *path = g_file_get_path(file);
  // TODO - 如果使用者选择了多文件保存那么这个路径就应该是一个目录，而不是文件。
  gtk_editable_set_text(GTK_EDITABLE(self->save_path), path);
}

static void save_choose(GtkButton *btn, NotePreferences *self) {
  GtkFileDialog *dialog = gtk_file_dialog_new();
  gtk_file_dialog_open(dialog, GTK_WINDOW(self), NULL, open_finish, self);
}

void create_empty_file(GFile *file) {
  g_file_create(file, G_FILE_CREATE_PRIVATE, NULL, NULL);
}

static void callback(GObject *source_object, GAsyncResult *res,
                     gpointer user_data) {
  NotePreferences *self = NOTE_PREFERENCES(user_data);
  GFile *file = G_FILE(source_object);
  GError *error = NULL;
  gboolean flag = false;
  if ((flag = g_file_mount_enclosing_volume_finish(file, res, &error))) {
    // g_file_find_enclosing_mount(file, NULL, NULL);
    gtk_label_set_markup(self->webdav_result,
                         "<span foreground='green'>OK</span>");
  } else if (error->code == 17) {
    gtk_label_set_markup(self->webdav_result,
                         "<span foreground='blue'>Already Connect</span>");
  } else {
    char *message =
        g_strconcat("<span foreground='red'>", error->message, "</span>", NULL);
    gtk_label_set_markup(self->webdav_result, message);
  }
  if (flag || error->code == 17) {
    GFile *note = g_file_get_child(file, "Note");
    if (g_file_query_exists(note, NULL)) {
      GFile *json = g_file_get_child(note, "data.json");
      if (g_file_query_exists(json, NULL)) {

      } else {
        create_empty_file(json);
      }
    } else {
      g_file_make_directory(note, NULL, NULL);
      GFile *json = g_file_get_child(note, "data.json");
      create_empty_file(json);
    }
  }
}

static void sync_webdav_test(GtkButton *btn, NotePreferences *self) {
  // GSocketAddress* address = g_list_nth (g_resolver_lookup_by_name (), 0);
  // FIXME - 不知道为什么无法获得填写的信息。gtk_accessible_text_get_contents:
  // assertion 'end >= start' failed错误。
  // TODO - webdav可以用gfile uri获得。
  const char *account =
      gtk_editable_get_text(GTK_EDITABLE(self->webdav_account));
  const char *addr = gtk_editable_get_text(GTK_EDITABLE(self->webdav_addr));
  const char *passwd =
      gtk_editable_get_text(GTK_EDITABLE(self->webdav_password));
  // const char* uri = g_uri_join_with_user(G_URI_FLAGS_NONE, addr, account,
  // passwd, NULL, NULL, 0, ".", NULL, NULL);
  char **head_addr = g_strsplit(addr, "://", 2);
  char *user_pass = g_strjoin(":", account, passwd, NULL);
  char *uri_path = g_strjoin("@", user_pass, head_addr[1], NULL);
  char *uri = g_strconcat("dav://", uri_path, NULL);
  g_print("%s\n", uri);
  GFile *file = g_file_new_for_uri(uri);
  GMountOperation *operation = g_mount_operation_new();
  g_mount_operation_set_password(operation, passwd);
  g_mount_operation_set_password_save(operation, G_PASSWORD_SAVE_PERMANENTLY);
  g_file_mount_enclosing_volume(file, G_MOUNT_MOUNT_NONE, operation, NULL,
                                callback, self);
}

static void note_preferences_init(NotePreferences *self) {
  gtk_widget_init_template(GTK_WIDGET(self));
  self->settings = g_settings_new("org.lion_claw.note");
  gtk_adjustment_set_value(self->height,
                           g_settings_get_uint(self->settings, "height"));
  gtk_adjustment_set_value(self->width,
                           g_settings_get_uint(self->settings, "width"));
  gtk_adjustment_set_value(self->title_num,
                           g_settings_get_uint(self->settings, "title-num"));
  gtk_adjustment_set_value(self->subtitle_num,
                           g_settings_get_uint(self->settings, "subtitle-num"));
  gtk_adjustment_set_value(
      self->subtitle_line,
      g_settings_get_uint(self->settings, "subtitle-line"));
  gtk_switch_set_active(
      self->custom_window_size,
      g_settings_get_boolean(self->settings, "custom-window-size"));
  gtk_switch_set_active(self->custom_font,
                        g_settings_get_boolean(self->settings, "custom-font"));
  gtk_switch_set_active(
      self->custom_editor_font,
      g_settings_get_boolean(self->settings, "custom-editor-font"));
  gtk_font_dialog_button_set_font_desc(
      self->custom_font_border,
      pango_font_description_from_string(
          g_settings_get_string(self->settings, "custom-font-border")));
  gtk_font_dialog_button_set_font_desc(
      self->custom_font_title,
      pango_font_description_from_string(
          g_settings_get_string(self->settings, "custom-font-title")));
  gtk_font_dialog_button_set_font_desc(
      self->custom_font_subtitle,
      pango_font_description_from_string(
          g_settings_get_string(self->settings, "custom-font-subtitle")));
  gtk_font_dialog_button_set_font_desc(
      self->custom_font_editor,
      pango_font_description_from_string(
          g_settings_get_string(self->settings, "custom-font-editor")));
  gtk_font_dialog_button_set_dialog(self->custom_font_border,
                                    gtk_font_dialog_new());
  gtk_font_dialog_button_set_dialog(self->custom_font_title,
                                    gtk_font_dialog_new());
  gtk_font_dialog_button_set_dialog(self->custom_font_subtitle,
                                    gtk_font_dialog_new());
  gtk_font_dialog_button_set_dialog(self->custom_font_editor,
                                    gtk_font_dialog_new());
  gtk_adjustment_set_value(self->lock_time,
                           g_settings_get_int(self->settings, "lock-time"));
  adw_preferences_row_set_title(
      ADW_PREFERENCES_ROW(self->save_path),
      g_settings_get_string(self->settings, "save-path"));
  gtk_editable_set_text(
      GTK_EDITABLE(self->webdav_account),
      g_settings_get_string(self->settings, "webdav-account"));
  gtk_editable_set_text(GTK_EDITABLE(self->webdav_addr),
                        g_settings_get_string(self->settings, "webdav-addr"));
  // gtk_editable_set_text(GTK_EDITABLE(self->webdav_password),
  //                       g_settings_get_string(self->settings,
  //                       "webdav-passwd"));
  g_signal_connect(self->webdav_test, "clicked", G_CALLBACK(sync_webdav_test),
                   self);
  g_signal_connect(self, "close-request", G_CALLBACK(note_preferences_close),
                   NULL);
  g_signal_connect(self->file_choose, "clicked", G_CALLBACK(save_choose), self);
}
