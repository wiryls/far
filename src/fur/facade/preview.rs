//! The main window for finding, replacing and previewing.

// References:
//
// [GUI development with Rust and GTK 4: Subclassing]
// (https://gtk-rs.org/gtk4-rs/git/book/gobject_subclassing.html)
//
// [gtk4-rs examples composite_template]
// (https://github.com/gtk-rs/gtk4-rs/tree/master/examples)
//
// [gtk4 getting started]
// (https://docs.gtk.org/gtk4/getting_started.html#a-menu)
//
// [Gtk4-tutorial: GtkMenuButton, accelerators, font, pango and gsettings]
// (https://github.com/ToshioCP/Gtk4-tutorial/blob/main/gfm/sec20.md)
//
// [Using GMenu]
// (https://developer.gnome.org/GMenu/)

// use glib::clone;
use gtk::subclass::prelude::*;
use gtk::{glib, prelude::*, CompositeTemplate};

#[derive(Debug, Default, CompositeTemplate)]
#[template(file = "preview.ui")]
pub struct Window {

    #[template_child]
    pub input_pattern: TemplateChild<gtk::Entry>,

    #[template_child]
    pub input_replace: TemplateChild<gtk::Entry>,

    #[template_child]
    pub button_rename: TemplateChild<gtk::Button>,
}

#[glib::object_subclass]
impl ObjectSubclass for Window {
    // should be same as what in *.ui file
    const NAME: &'static str = "PreviewWindow";
    // point to the wrappered type
    type Type = super::PreviewWindow;
    // this windows is derived from gtk::ApplicationWindow
    type ParentType = gtk::ApplicationWindow;

    fn class_init(klass: &mut Self::Class) {
        Self::bind_template(klass);

        // bind actions from the ui file to our window
        klass.install_action("win.quit", None, move |this, _action_name, _action_target| {
            let this = Window::from_instance(this);
            this.instance().close();
        });

        // [install action]
        // (https://github.com/gtk-rs/gtk4-rs/blob/6c296e5763faf8e3e967c457b318b8a8fbd45303/examples/video_player/video_player_window/imp.rs#L47)
    }

    fn instance_init(o: &glib::subclass::InitializingObject<Self>) {
        o.init_template();
    }
}

impl ObjectImpl for Window {}
impl WidgetImpl for Window {}
impl WindowImpl for Window {}
impl ApplicationWindowImpl for Window {}
