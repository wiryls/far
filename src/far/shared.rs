/// Stringer is something just like String type.
pub trait Stringer : AsRef<str> {
    fn append<'a>(&mut self, rhs: &'a str);
}

impl<T> Stringer for T
where
    for<'a> T: AsRef<str> + std::ops::AddAssign<&'a str>
{
    fn append<'a>(&mut self, rhs: &'a str) {
        self.add_assign(rhs)
    }
}
