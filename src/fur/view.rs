use std::rc::Rc;
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

    const ID : &'static str = "com.github.wiryls.far";

    pub fn run(&self) {
        self.app.run();
    }

    pub fn new() -> Self {
        let ctx = Rc::new(Context{});
        let app = gtk::Application::builder()
            .application_id(View::ID)
            .flags(Default::default())
            .build();

        // [signals]
        // (https://wiki.gnome.org/HowDoI/GtkApplication)
        app.connect_activate(
            clone!(@weak ctx => move
                |app| View::bind(ctx.as_ref(), app) ));

        Self{ctx, app}
    }

    fn bind(_ctx: &Context, app: &gtk::Application) {
        View::load();
        PreviewWindow::new(app).show();
    }

    fn load() {
        const RES: &'static [u8] = include_bytes!("res.gresource");
        let res = glib::Bytes::from(RES);
        let res = gio::Resource::from_data(&res).unwrap();
        gio::resources_register(&res);
    }
}
