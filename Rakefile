require "bundler/gem_tasks"
require 'rake/extensiontask'

Rake::ExtensionTask.new('visualize_helper') do |ext|
  ext.lib_dir = "lib/visualize_helper"
end  

task :default => [:compile, :spec]
task :test => [:compile, :spec]

