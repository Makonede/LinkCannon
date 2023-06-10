module.exports = {
  plugins: [
    [
      'postcss-import',
      {
        path: [
          'node_modules'
        ]
      }
    ],
    [
      'tailwindcss',
      {}
    ],
    [
      'autoprefixer',
      {}
    ],
    [
      'postcss-preset-env',
      {
        features: {
          'nesting-rules': false
        }
      }
    ]
  ]
};
