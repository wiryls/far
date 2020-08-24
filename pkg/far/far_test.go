package far_test

import (
	"strconv"
	"testing"

	"github.com/stretchr/testify/assert"

	"github.com/wiryls/far/pkg/far"
)

func TestRegexpFindAndReplace(t *testing.T) {
	assert := assert.New(t)

	type Case struct {
		pattern  string
		template string
		samples  [][2]string
	}

	cases := []Case{
		{
			``,
			``,
			[][2]string{
				{
					"",
					"",
				},
				{
					"QAQ",
					"QAQ",
				},
			},
		},
		{
			`\.\.`,
			`.`,
			[][2]string{
				{
					"..",
					".",
				},
				{
					"......",
					"...",
				},
				{
					"._..___....",
					"._.___..",
				},
				{
					"OTZ",
					"OTZ",
				},
			},
		},
		{
			`(\d+)\+(\d+)`,
			`$2+$1`,
			[][2]string{
				{
					"",
					"",
				},
				{
					"1+2",
					"2+1",
				},
				{
					"?+1",
					"?+1",
				},
				{
					"1+?",
					"1+?",
				},
				{
					"1+?+3+4",
					"1+?+4+3",
				},
				{
					"?+3+3+4",
					"?+3+3+4",
				},
				{
					"1+1+1+1+1",
					"1+1+1+1+1",
				},
				{
					"1+2+3+4+5+6+7+8",
					"2+1+4+3+6+5+8+7",
				},
			},
		},
		{
			`(?P<hello>\d+)`,
			``,
			[][2]string{
				{
					"",
					"",
				},
				{
					"1+2",
					"+",
				},
				{
					"?+1",
					"?+",
				},
				{
					"h1z1",
					"hz",
				},
				{
					"he110w0r1d",
					"hewrd",
				},
				{
					"1234567890",
					"",
				},
			},
		},
	}

	far := far.New()
	for i := range cases {
		c := cases[i]
		m := "CASE " + strconv.Itoa(i)

		assert.NotNil(far, m)
		assert.NoError(far.SetPattern(c.pattern))
		assert.NoError(far.SetTemplate(c.template))
		assert.EqualValues(c.pattern, far.Pattern())
		assert.EqualValues(c.template, far.Template())

		for i, text := range c.samples {
			m := m + "#" + strconv.Itoa(i)
			o := far.See(text[0])
			assert.Equal(text[0], o.Old(), m)
			assert.Equal(text[1], o.New(), m)
			assert.Equal(text[1], o.String(), m)
			assert.Equal(text[0] == text[1], o.IsSame(), m)
		}
	}
}
