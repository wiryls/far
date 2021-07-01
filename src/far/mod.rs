mod diff;
pub use self::diff::{Change, Diff, DiffIterator};

mod sofar;
pub use self::sofar::Far;

mod faregex;
pub use self::faregex::Faregex;

// note: submodule management
// https://stackoverflow.com/a/49476448
