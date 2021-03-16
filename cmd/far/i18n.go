package main

type texts struct {
	locale string
}

func (t *texts) PatternIsInvalid() string {
	switch t.locale {
	default:
		return "It is not a valid regex pattern"
	}
}

func (t *texts) TemplateIsInvalid() string {
	switch t.locale {
	default:
		return "Do not use '\\'"
	}
}
