use far::{Far, Faregex, Change};

#[test]
fn when_faregex_is_empty() {
    let mut far = Faregex::new();
    assert_eq!(far.get_pattern (), "");
    assert_eq!(far.get_template(), "");

    assert!(far.set_pattern("").is_none());
    far.set_template("");
    assert_eq!(far.get_pattern (), "");
    assert_eq!(far.get_template(), "");

    let src = "hello";
    let dst = far.see(src);
    assert!   (dst.is_same());
    assert_eq!(dst.target(src), src);

    for c in dst.iter(src) {
        match c {
            Change::Retain(s) => assert_eq!(s, src),
            _ => assert!(false),
        }
    }
}

#[test]
fn when_faregex_return_values() {
    let mut far = Faregex::new();

    assert!(far.set_pattern("[a-z]").is_none());
    assert_eq!(far.get_pattern (), "[a-z]");

    far.set_template("");
    assert_eq!(far.get_template(), "");

    let src = "he110 w0r1d";
    let dst = far.see(src);
    assert!(!dst.is_same());
    assert_eq!(dst.target(src), "110 01");

    for (i, c) in dst.iter(src).enumerate() {
        assert!(match i {
            0 => matches!(c, Change::Delete("he")),
            1 => matches!(c, Change::Retain("110 ")),
            2 => matches!(c, Change::Delete("w")),
            3 => matches!(c, Change::Retain("0")),
            4 => matches!(c, Change::Delete("r")),
            5 => matches!(c, Change::Retain("1")),
            6 => matches!(c, Change::Delete("d")),
            _ => false,
        });
    }
}

#[test]
fn when_faregex_is_not_empty() {
    for (mut f, n) in vec![
        (Box::new(Faregex::new()) as Box<dyn Far>, "Faregex")
    ] {
        for (i, c) in vec![(
                "", "",
                vec![
                    ["", ""],
                    ["QAQ", "QAQ"],
                ]
            ), (
                r"\.\.", r".",
                vec![
                    ["", ""],
                    ["..", "."],
                    ["......", "..."],
                    ["._..___....", "._.___.."],
                    ["OTZ", "OTZ"],
                ]
            ), (
                r"(\d+)\+(\d+)", r"$2+$1",
                vec![
                    ["", ""],
                    ["1+2", "2+1"],
                    ["?+1", "?+1"],
                    ["1+?", "1+?"],
                    ["1+?+3+4", "1+?+4+3"],
                    ["?+3+3+4", "?+3+3+4"],
                    ["1+1+1+1+1","1+1+1+1+1"],
                    ["1+2+3+4+5+6+7+8","2+1+4+3+6+5+8+7"],
                ]
            ), (
                r"(?P<hello>\d+)", r"",
                vec![
                    ["", ""],
                    ["1+2", "+"],
                    ["?+1", "?+"],
                    ["h1z1", "hz"],
                    ["he110w0r1d", "hewrd"],
                    ["1234567890", ""],
                ]
            ),
        ].iter().enumerate() {
            assert!(f. set_pattern(c.0).is_none(), "CASE({}, {})", n, i);
            assert!(f.set_template(c.1).is_none(), "CASE({}, {})", n, i);
            assert_eq!(f. get_pattern(), c.0, "CASE({}, {})", n, i);
            assert_eq!(f.get_template(), c.1, "CASE({}, {})", n, i);

            for (j, s) in c.2.iter().enumerate() {
                let o = f.see(s[0]);
                assert_eq!(o.target(s[0]), s[1], "CASE({}, {}, {})", n, i, j);
                assert_eq!(o.is_same(), s[0] == s[1], "CASE({}, {}, {})", n, i, j);
            }
        }
    }
}
