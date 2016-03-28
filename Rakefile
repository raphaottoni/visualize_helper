require "bundler/gem_tasks"
require 'rake/extensiontask'
require "rspec/core/rake_task"

Rake::ExtensionTask.new('visualize_helper') do |ext|
  ext.lib_dir = "lib/visualize_helper"
end  

RSpec::Core::RakeTask.new

task :default => [:compile, :spec]
task :test => [:compile, :spec]

