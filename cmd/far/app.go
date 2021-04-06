package main

import "github.com/wiryls/pkg/runner"

type App struct {
	main runner.Runner
}

// New app, init this application.
func (a *App) New() (err error) {
	a.main, err = NewFront()
	return
}

// Run this application.
func (a *App) Run() error {
	return a.main.Run()
}
