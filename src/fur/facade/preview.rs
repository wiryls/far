//! The main window for finding, replacing and previewing.

// NOTE:
// see [examples](https://github.com/gtk-rs/gtk4-rs/tree/master/examples) for
// deatils.

use glib::clone;
use gtk::subclass::prelude::*;
use gtk::{glib, prelude::*, CompositeTemplate};

use super::PreviewMenuButton;

#[derive(Debug, Default, CompositeTemplate)]
#[template(file = "preview_window.ui")]
pub struct Window {
    #[template_child]
    pub menubutton: TemplateChild<PreviewMenuButton>,
}

#[glib::object_subclass]
impl ObjectSubclass for Window {
    // should be same as what in *.ui file
    const NAME: &'static str = "PreviewWindow";
    // point to the wrappered type
    type Type = super::PreviewWindow;
    // this windows is derived from gtk::ApplicationWindow
    type ParentType = gtk::ApplicationWindow;

    fn class_init(t: &mut Self::Class) {
        Self::bind_template(t);
    }

    fn instance_init(o: &glib::subclass::InitializingObject<Self>) {
        o.init_template();
    }
}

impl ObjectImpl for Window {}
impl WidgetImpl for Window {}
impl WindowImpl for Window {}
impl ApplicationWindowImpl for Window {}

#[derive(Debug, Default, CompositeTemplate)]
#[template(file = "preview_menu.ui")]
pub struct MenuButton {
    #[template_child]
    pub toggle: TemplateChild<gtk::ToggleButton>,
    #[template_child]
    pub popover: TemplateChild<gtk::Popover>,
}

#[glib::object_subclass]
impl ObjectSubclass for MenuButton {
    const NAME: &'static str = "PreviewMenuButton";
    type Type = super::PreviewMenuButton;
    type ParentType = gtk::Widget;

    fn class_init(klass: &mut Self::Class) {
        Self::bind_template(klass);
    }

    fn instance_init(obj: &glib::subclass::InitializingObject<Self>) {
        obj.init_template();
    }
}

impl ObjectImpl for MenuButton {
    fn constructed(&self, obj: &Self::Type) {
        self.parent_constructed(obj);

        let popover = &*self.popover;
        self.toggle
            .connect_toggled(glib::clone!(@weak popover => move |toggle| {
                if toggle.is_active() {
                    popover.popup();
                }
            }));

        let toggle = &*self.toggle;
        self.popover
            .connect_closed(glib::clone!(@weak toggle => move |_| {
                toggle.set_active(false);
            }));
    }

    // Needed for direct subclasses of GtkWidget;
    // Here you need to unparent all direct children
    // of your template.
    fn dispose(&self, obj: &Self::Type) {
        while let Some(child) = obj.first_child() {
            child.unparent();
        }
    }
}

impl WidgetImpl for MenuButton {
    fn size_allocate(&self, widget: &Self::Type, width: i32, height: i32, baseline: i32) {
        self.parent_size_allocate(widget, width, height, baseline);
        self.popover.present();
    }
}
