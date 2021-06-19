use gtk::{glib, gio};
use gtk::subclass::prelude::*;

#[derive(Debug)]
pub struct List;

#[glib::object_subclass]
impl ObjectSubclass for List {
    const NAME: &'static str = "com.github.wiryls.far.fur.list";
    type Type = super::List;
    type ParentType = glib::Object;
    type Interfaces = (gio::ListModel,);

    fn class_init(klass: &mut Self::Class) {}

    fn new() -> Self {
        Self {}
    }
}

impl ObjectImpl for List {

}

impl ListModelImpl for List {

    fn item_type(&self, list_model: &Self::Type) -> glib::Type {
        todo!()
    }

    fn n_items(&self, list_model: &Self::Type) -> u32 {
        todo!()
    }

    fn item(&self, list_model: &Self::Type, position: u32) -> Option<glib::Object> {
        todo!()
    }
}
