use gtk::{gio, glib};

glib::wrapper! {
    pub struct PreviewMenuButton(ObjectSubclass<preview::MenuButton>)
        @extends gtk::Widget;
}
