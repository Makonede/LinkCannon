/*
eslint.config.js - ESLint configuration file.
Copyright (C) 2023  Makonede

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


import js from '@eslint/js'

import globals from 'globals'

import typescript from '@typescript-eslint/eslint-plugin'
import typescriptParser from '@typescript-eslint/parser'

import svelte from 'eslint-plugin-svelte'
import svelteParser from 'svelte-eslint-parser'

import prettier from 'eslint-config-prettier'
import prettierSvelte from 'prettier-plugin-svelte'


export default [
  js.configs.recommended,
  typescript.configs.recommended,
  svelte.configs.recommended,
  svelte.configs.prettier,
  prettier,
  {
    languageOptions: {
      globals: {
        ...globals.node,
        ...globals.browser
      },
      parser: typescriptParser,
      parserOptions: {
        sourceType: 'module',
        extraFileExtensions: ['.svelte']
      }
    },
    plugins: {
      prettierSvelte: prettierSvelte,
      typescript: typescript,
      svelte: svelte
    }
  },
  {
    files: ['*.svelte'],
    languageOptions: {
      parser: svelteParser,
      parserOptions: {
        parser: typescriptParser
      }
    }
  }
]
