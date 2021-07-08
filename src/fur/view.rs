use std::rc::{Rc, Weak};
use gtk::prelude::*;
use gtk::{gio, glib, glib::clone};
use crate::fur::fiber::{Item, List};

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

        let menu = gtk::MenuButton::builder()
            .icon_name("open-menu-symbolic")
            .build();

        let head = gtk::HeaderBar::builder()
            .build();

        let win = gtk::ApplicationWindow::builder()
            .application(app)
            .default_width(350)
            .default_height(70)
            .title("F A R")
            // .child(&container)
            .build();

        head.pack_start(&menu);
        win.set_titlebar(Some(&head));
        win.show();
    }
}
