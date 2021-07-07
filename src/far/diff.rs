use super::Stringer;

/// Patch is an internal type. It records a single step about transforming
/// the source string to target.
/// For now there is no small string optimization in Rust. We decide to keep
/// some ranges instead of new strings.
#[derive(Clone, Debug)]
pub(in crate::far) enum Patch<T>
where
    T : Stringer
{
    Retain(std::ops::Range<usize>),
    Delete(std::ops::Range<usize>),
    Insert(T),
}

/// Diff is something related to transform a source string to its target
/// string.
#[derive(Clone, Debug, Default)]
pub struct Diff<T>(Vec<Patch<T>>)
where
    T : Stringer;

impl<T> Diff<T>
where
    T : Stringer
{

    pub(in crate::far) fn from_vec(src: Vec<Patch<T>>) -> Self {
        let mut dst : Vec<Patch<T>> = Vec::with_capacity(src.len());

        for rhs in src.into_iter() {
            use Patch::{Retain, Delete, Insert};
            match dst.last_mut() {
                Some(lhs) => match (lhs, &rhs) {
                    (Retain(l), Retain(r)) |
                    (Delete(l), Delete(r)) if l.end == r.start => l.end = r.end,
                    (Insert(l), Insert(r)) => l.append(r.as_ref()),
                    _ => dst.push(rhs),
                }
                None => dst.push(rhs),
            };
        }

        dst.shrink_to_fit();
        Self(dst)
    }

    pub fn iter<'a>(&'a self, source : &'a str) -> DiffIterator<'a, T> {
        DiffIterator{
            index: 0,
            mapper: self,
            source,
        }
    }

    pub fn is_same(&self) -> bool {
        return self.0.is_empty();
    }

    pub fn target(&self, source: &str) -> String {
        self.iter(source)
            .filter_map(|x| match x {
                Change::Retain(s) => Some(s),
                Change::Delete(_) => None,
                Change::Insert(s) => Some(s)})
            .collect()
    }
}

/// An iterator to walk through Diff.
#[derive(Clone, Debug)]
pub struct DiffIterator<'a, T>
where
    T: Stringer
{
    index: usize,
    mapper: &'a Diff<T>,
    source: &'a str,
}

impl<'a, T> Iterator for DiffIterator<'a, T>
where
    T: Stringer
{

    type Item = Change<'a>;

    fn next(&mut self) -> Option<Self::Item> {
        let out = match self.mapper.0.get(self.index) {
            Some(d) => match d {
                Patch::Retain(r) => self.source.get(r.clone()).map(Change::Retain),
                Patch::Delete(r) => self.source.get(r.clone()).map(Change::Delete),
                Patch::Insert(s) => Some(Change::Insert(s.as_ref())),
            }
            None => match self.index {
                0 => Some(Change::Retain(&self.source)),
                _ => None
            }
        };
        self.index += out.is_some() as usize;
        out
    }
}

/// Change is an operation to Retain, Delete or Insert a string.
/// It references to source string and a Patch.
#[derive(Clone, Debug)]
pub enum Change<'a> {
    Retain(&'a str),
    Delete(&'a str),
    Insert(&'a str),
}
