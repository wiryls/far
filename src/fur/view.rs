use super::fiber::{Item, List};
use gtk::prelude::*;
use gtk::{gio, glib};

pub struct MainView {
    app     : gtk::Application,
    pattern : gtk::Entry,
    tempalte: gtk::Entry,
    rename  : gtk::Button,
    model   : List,
    table   : gtk::ColumnView,
}

impl MainView {

    const ID : &'static str = "com.github.wiryls.far.testing";

    pub fn new() -> Self {
        let this = Self {

            app: gtk::Application::builder()
                .application_id(MainView::ID)
                .flags(Default::default())
                .build(),

            pattern: gtk::Entry::builder()
                .build(),

            tempalte: gtk::Entry::builder()
                .build(),

            rename: gtk::Button::builder()
                .build(),

            model: List::default(),

            table: gtk::ColumnView::builder()
                .build(),
        };

        // this.app.connect_activate(|_| MainView::connect(&this));
        // this.app.run();

        this
    }

    fn connect(&self) {

    }
}
