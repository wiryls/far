use super::fiber::{Item, List};

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

    // pub fn new() -> Self {
    //     let app = gtk::Application::new(Some(MainView::ID), Default::default());
    //     app.connect_activate(build_ui);
    //     app.run();
    // }
}
