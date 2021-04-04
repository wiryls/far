package main

import "github.com/wiryls/pkg/runner"

type app struct {
	main runner.Runner
}

// New app, init this application.
func (a *app) New() error {
	a.main = NewViewModel()
	return nil
}

// Run this application.
func (a *app) Run() error {
	return a.main.Run()
}
