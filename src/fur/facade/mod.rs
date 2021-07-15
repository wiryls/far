mod import;
mod rename;
mod preview;

use gtk::subclass::prelude::*;
use gtk::{gio, glib};
use crate::fur::Context;

glib::wrapper! {
    pub struct PreviewWindow(ObjectSubclass<preview::Window>)
        @extends gtk::Widget,
                 gtk::Window,
                 gtk::ApplicationWindow,
        @implements gio::ActionMap,
                    gio::ActionGroup;
}

impl PreviewWindow {

    pub fn new<P>(app: &P) -> Self
    where
        P: glib::IsA<gtk::Application>
    {
        glib::Object::new(&[("application", app)])
            .expect("failed to create PreviewWindow")
    }

    pub fn bind(&self, ctx: &Context) {
        preview::Window::from_instance(self).bind(ctx);
    }
}
