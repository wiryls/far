use gtk::prelude::*;
use gtk::{gio, glib};

mod fur;

fn main() {
    let view = fur::View::new();
    view.run();
}

fn build_ui(application: &gtk::Application) {

    let factory = gtk::SignalListItemFactory::new();
    factory.connect_setup(move |_, item| {
        item.set_child(Some(&gtk::Label::new(None)));
    });
    factory.connect_bind(move |_, item| {
        // item.item().unwrap().downcast::<>()
    });

    let s = gtk::StringList::new(&["A", "B"]);

    let model = gio::ListStore::new(glib::String::static_type());

    let model = gtk::MultiSelection::new(Some(&model));

    let column = gtk::ColumnViewColumn::builder()
        .title("hello")
        .factory(&factory)
        .build();

    let list = gtk::ColumnView::builder()
        .model(&model)
        .build();
    list.append_column(&column);

    let container = gtk::Box::builder()
        .orientation(gtk::Orientation::Vertical)
        .spacing(8)
        .build();

    container.append(&gtk::Button::builder()
        .label("Click me!")
        .build()
    );
    container.append(&list);

    gtk::ApplicationWindow::builder()
        .application(application)
        .title("First GTK Program")
        .default_width(350)
        .default_height(70)
        .child(&container)
        .build()
        .show();
}
