mod diff;
mod adapte;
pub use self::diff::{Stringer, Diff, DiffIterator, Change};

mod sofar;
pub use self::sofar::Far;

mod faregex;
pub use self::faregex::Faregex;

// note: submodule management
// https://stackoverflow.com/a/49476448
