require 'rubygems'
require 'rake'

spec = Gem::Specification.new do |s|
  s.name = "rmotion"
  s.version = "0.0.1"
  s.summary = "RMotion provides a simple interface to build motion detection software in ruby."
  s.description = "RMotion provides a simple interface to build motion detection software in ruby. Many kind of informations about moving entities will be available to applications, and the algorithm is heavily customizable since different cameras record different levels of noise."
  s.licenses = ['GPL']

  s.author = "Riccardo Cecolin"
  s.email = "rikiji@playkanji.com"
  s.homepage = "http://www.rikiji.it"

  s.files = FileList["examples/*.rb","ext/rmotion/*.rb","ext/rmotion/*.h","ext/rmotion/*.c","lib/*","LICENSE","README"]
  s.extensions = FileList["ext/rmotion/extconf.rb"]
end

