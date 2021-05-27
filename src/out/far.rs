use regex::{Regex, Error};
use super::{Diff, Diffs};

pub trait Far
{
    fn see         (&self, text : &str) -> Diffs;
    fn is_empty    (&self)              -> bool;
    fn get_pattern (&self)              -> &str;
    fn set_pattern (&mut self, pattern  : &str) -> Option<Error>;
    fn get_template(&self)              -> &str;
    fn set_template(&mut self, template : &str) -> Option<Error>;
}

pub struct Faregex
{
    pattern  : Option<Regex>,
    template : String,
}

impl Faregex
{
    pub const fn new() -> Self
    {
        Faregex
        {
            pattern : None,
            template: String::new(),
        }
    }

    pub fn see(&self, text : &str) -> Diffs
    {
        match &self.pattern
        {
            None     => vec![Diff::Retain(text.to_owned())],
            Some(re) =>
            {
                let mut diff : Diffs = Vec::new();
                let mut last : usize = 0;
                for cap in re.captures_iter(text)
                {
                    let m = match cap.get(0)
                    {
                        None    => continue,
                        Some(m) => m,
                    };

                    let     ret = &text[last..m.start()];
                    let     del = &text[m.range()];
                    let mut ins = String::new();
                    cap.expand(self.template.as_str(), &mut ins);

                    if del != ins
                    {
                        if !ret.is_empty()
                        {
                            diff.push(Diff::Retain(ret.to_owned()));
                        }
                        if !del.is_empty()
                        {
                            diff.push(Diff::Delete(del.to_owned()));
                        }
                        if !ins.is_empty()
                        {
                            diff.push(Diff::Insert(ins));
                        }
                        last = m.end();
                    }
                }
                if last != text.len()
                {
                    diff.push(Diff::Retain(text[last..].to_owned()));
                }
                diff
            },
        }
    }

    pub fn is_empty(&self) -> bool
    {
        self.pattern.is_none()
    }

    pub fn get_pattern(&self) -> &str
    {
        match &self.pattern
        {
            None    => &"",
            Some(p) => p.as_str(),
        }
    }

    pub fn set_pattern(&mut self, pattern : &str) -> Option<Error>
    {
        match Regex::new(pattern)
        {
            Ok(r)  => {self.pattern = Some(r); None},
            Err(e) => Some(e),
        }
    }

    pub fn get_template(&self) -> &str
    {
        &self.template[..]
    }

    pub fn set_template(&mut self, template : &str) -> Option<Error>
    {
        self.template = template.to_owned();
        None
    }
}

impl Far for Faregex
{
    fn see(&self, text : &str) -> Diffs
    {
        self.see(text)
    }

    fn is_empty(&self) -> bool
    {
        self.is_empty()
    }

    fn get_pattern(&self) -> &str
    {
        self.get_pattern()
    }

    fn get_template(&self) -> &str
    {
        self.get_template()
    }

    fn set_pattern(&mut self, pattern : &str) -> Option<Error>
    {
        self.set_pattern(pattern)
    }

    fn set_template(&mut self, template : &str) -> Option<Error>
    {
        self.set_template(template)
    }
}
