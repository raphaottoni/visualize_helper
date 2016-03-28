# coding: utf-8
lib = File.expand_path('../lib', __FILE__)
$LOAD_PATH.unshift(lib) unless $LOAD_PATH.include?(lib)
require 'visualize_helper/version'

Gem::Specification.new do |spec|
  spec.name          = "visualize_helper"
  spec.version       = VisualizeHelper::VERSION
  spec.authors       = ["Raphael Ottoni Santiago Machado de Faria"]
  spec.email         = ["rapha.ottoni@gmail.com"]
  spec.summary       = %q{Common methods used in visualize.dcc.ufmg.br}
  spec.description   = %q{Common methods used in visualize.dcc.ufmg.br}
  spec.homepage      = ""
  spec.license       = "MIT"
  spec.required_ruby_version = '>=  2.1.5'

  spec.files         = `git ls-files -z`.split("\x0")
  spec.executables   = spec.files.grep(%r{^bin/}) { |f| File.basename(f) }
  spec.test_files    = spec.files.grep(%r{^(test|spec|features)/})
  spec.require_paths = ["lib"]

  spec.extensions = %w[ext/visualize_helper/extconf.rb]

  spec.add_development_dependency "bundler", "~> 1.7.3"
  spec.add_development_dependency "rake", "~> 10.3.2"
  spec.add_development_dependency "rake-compiler", "~> 0.9.3"
  spec.add_development_dependency "rspec", "~> 3.1.0"
  spec.add_development_dependency "guard-rspec", "~> 4.3.1"
end
