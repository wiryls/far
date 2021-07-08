use super::diff::Stringer;

impl<T> Stringer for T
where
    // notes for Higher-Rank Trait Bounds:
    // https://doc.rust-lang.org/nomicon/hrtb.html
    // https://github.com/rust-lang/rfcs/blob/master/text/0387-higher-ranked-trait-bounds.md
    for<'a> T: AsRef<str> + std::ops::AddAssign<&'a str>
{
    fn append<'a>(&mut self, rhs: &'a str) {
        self.add_assign(rhs)
    }
}
