package main

type Resource func(string) string

func (r *Resource) Set(locale string) Resource {

	v := map[string]map[string]string{
		"main_title": {
			"en": "FAR",
		},
		"main_rename": {
			"en": "Rename",
			"zh": "重命名"},
		"main_input_pattern": {
			"en": "Pattern",
			"zh": "替换"},
		"main_input_template": {
			"en": "Template",
			"zh": "模板"},
		"main_column_stat": {
			"en": "Stat",
			"zh": "状态"},
		"main_column_name": {
			"en": "Stat",
			"zh": "名字"},
		"main_column_path": {
			"en": "Stat",
			"zh": "路径"},
	}

	*r = func(key string) string {
		m, ok := v[key]
		if ok {
			for _, locale := range []string{string(locale), "en"} {
				v, ok := m[locale]
				if ok {
					return v
				}
			}
		}
		return "?"
	}

	return *r
}
