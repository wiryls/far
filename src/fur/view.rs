use std::rc::{Rc, Weak};
use gtk::prelude::*;
use gtk::{gio, glib, glib::clone};
use crate::fur::fervor::{Item, List};
use crate::fur::facade::PreviewWindow;

pub struct View {
    ctx: Rc<Context>,
    app: gtk::Application,
}

struct Context {

}

impl View {

    const ID : &'static str = "com.github.wiryls.far.testing";

    pub fn new() -> Self {
        let ctx = Rc::new(Context{});
        let app = gtk::Application::builder()
            .application_id(View::ID)
            .flags(Default::default())
            .build();

        // [signals]
		// (https://wiki.gnome.org/HowDoI/GtkApplication)
        app.connect_activate(
            clone!(@weak ctx =>
                move |app| View::build_main_window(ctx, app)));

        Self{ctx, app}
    }

    pub fn run(&self) {
        self.app.run();
    }

    fn build_main_window(ctx: Rc<Context>, app: &gtk::Application) {

        // let builder = gtk::Builder::from_resource("");
        // let model : gio::MenuModel = builder.object("menu").unwrap();

        PreviewWindow::new(app).show();
    }
}
