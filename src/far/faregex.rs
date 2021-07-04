//! This file is based on:
//! https://github.com/wiryls/far/blob/46271a4a1dfb69c321aea4564337b838132b775f/pkg/far/faregex.go
use regex::{Regex, Error};
use super::diff::{Diff, Patch};

#[derive(Clone, Debug)]
pub struct Faregex {
    pattern  : Option<Regex>,
    template : String,
}

/// An find-and-replace tool base on regexp.
impl Faregex {

    /// Create a Faregex.
    pub const fn new() -> Self {
        Faregex {
            pattern : None,
            template: String::new(),
        }
    }

    /// See the difference.
    pub fn see(&self, text : &str) -> Diff {
        match &self.pattern {
            None => Diff::default(),
            Some(regexp) => {
                let mut last : usize = 0;
                let mut diff = Vec::new();
                for cap in regexp.captures_iter(text) {
                    // Why is the Item of SubCaptureMatches.Iterator an
                    // Option<Match> ???
                    // https://docs.rs/regex/1.5.4/regex/struct.SubCaptureMatches.html#associatedtype.Item
                    let m = match cap.get(0) {
                        None    => continue,
                        Some(m) => m,
                    };

                    let     ret = &text[last..m.start()];
                    let     del = &text[m.range()];
                    let mut ins = String::new();
                    cap.expand(self.template.as_str(), &mut ins);

                    if del != ins {
                        if !ret.is_empty() {
                            diff.push(Patch::Retain(last..m.start()));
                        }
                        if !del.is_empty() {
                            diff.push(Patch::Delete(m.range()));
                        }
                        if !ins.is_empty() {
                            diff.push(Patch::Insert(ins));
                        }
                        last = m.end();
                    }
                }
                if 0 != last && last != text.len() {
                    diff.push(Patch::Retain(last..text.len()));
                }
                Diff::from_vec(diff)
            },
        }
    }

    /// is it always returns `Diff::Retain(String)`
    pub fn is_empty(&self) -> bool {
        self.pattern.is_none()
    }

    /// set a pattern for regex.
    pub fn set_pattern(&mut self, pattern : &str) -> Option<Error> {
        match Regex::new(pattern) {
            Ok (r) => {self.pattern = Some(r); None},
            Err(e) => Some(e),
        }
    }

    /// read the pattern for regex.
    pub fn get_pattern(&self) -> &str {
        match &self.pattern {
            None    => &"",
            Some(p) => p.as_str(),
        }
    }

    /// set a template for replacing.
    pub fn set_template(&mut self, template : &str) {
        self.template = template.to_owned();
    }

    /// get the template for replacing.
    pub fn get_template(&self) -> &str {
        &self.template[..]
    }
}
