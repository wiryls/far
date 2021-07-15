use std::rc::Rc;
use gtk::prelude::*;
use gtk::{gio, glib, glib::clone};
use crate::fur::fervor::List;
use crate::fur::facade::PreviewWindow;

pub struct Context {

}

pub struct View(gtk::Application);

impl View {

    pub fn run(&self) {
        self.0.run();
    }

    pub fn new(ctx: Rc<Context>) -> Self {
        const ID : &'static str = "com.github.wiryls.far";

        let app = gtk::Application::builder()
            .application_id(ID)
            .flags(Default::default())
            .build();

        app.connect_activate(
            clone!(@weak ctx => move
                |app| View::bind(ctx.as_ref(), app) ));

        // Reference:
        //
        // [signals]
        // (https://wiki.gnome.org/HowDoI/GtkApplication)

        Self(app)
    }

    fn bind(ctx: &Context, app: &gtk::Application) {
        // prepare
        View::load();

        // build window
        let win = PreviewWindow::new(app);
        println!("binding win");
        win.bind(ctx);
        win.show();
    }

    fn load() {
        use std::sync::Once;
        static ONESHOT: Once = Once::new();
        ONESHOT.call_once(|| {
            const RES: &'static [u8] = include_bytes!("res.gresource");
            let res = glib::Bytes::from(RES);
            let res = gio::Resource::from_data(&res).unwrap();
            gio::resources_register(&res);
        });
    }
}
