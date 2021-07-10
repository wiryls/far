mod import;
mod rename;
mod preview;

// use gtk::subclass::prelude::*;
use gtk::{gio, glib};

glib::wrapper! {
    pub struct PreviewWindow(ObjectSubclass<preview::Window>)
        @extends gtk::Widget,
                 gtk::Window,
                 gtk::ApplicationWindow,
        @implements gio::ActionMap,
                    gio::ActionGroup;
}

impl PreviewWindow {
    pub fn new<P: glib::IsA<gtk::Application>>(app: &P) -> Self {
        glib::Object::new(&[("application", app)])
            .expect("failed to create PreviewWindow")
    }
}
