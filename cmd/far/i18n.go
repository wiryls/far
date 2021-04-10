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
		"main_tree_column_stat": {
			"en": "Stat",
			"zh": "状态"},
		"main_tree_column_name": {
			"en": "Name",
			"zh": "名字"},
		"main_tree_column_path": {
			"en": "Path",
			"zh": "路径"},
		"main_tree_menu_delete": {
			"en": "Delete",
			"zh": "移除"},
		"main_tree_menu_clear": {
			"en": "Clear",
			"zh": "清空"},
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
