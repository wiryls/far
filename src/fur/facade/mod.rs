mod import;
mod rename;
mod preview;

use gtk::{gio, glib};
use gtk::subclass::prelude::*;
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

    pub fn new<P>(ctx: &Context, app: &P) -> Self
    where
        P: glib::IsA<gtk::Application>
    {
        glib::Object::new(&[("application", app)])
            .map(|x: Self| x.setup(ctx))
            .expect("failed to create PreviewWindow")
    }

    fn setup(self, ctx: &Context) -> Self {
        preview::Window::from_instance(&self).setup(ctx);
        self
    }
}
