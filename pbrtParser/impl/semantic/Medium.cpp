#include "SemanticParser.h"

namespace pbrt {

void SemanticParser::extractMediumInterface(Shape::SP geom, pbrt::syntactic::Shape shape) {
	if (!shape.attributes->mediumInterface.first.empty()) {
		geom->mediumInterface->inside = findOrCreateMedium(
			shape.attributes->findNamedMedium(shape.attributes->mediumInterface.first));
	}
	if (!shape.attributes->mediumInterface.second.empty()) {
		geom->mediumInterface->inside = findOrCreateMedium(
			shape.attributes->findNamedMedium(shape.attributes->mediumInterface.second));
	}
}

Medium::SP SemanticParser::findOrCreateMedium(pbrt::syntactic::Medium::SP in) {
	if (!in) return Medium::SP();
		
	if (mediumMapping.find(in) != mediumMapping.end()) 
		return mediumMapping[in];
	
	mediumMapping[in] = createMediumFrom(in);
	return mediumMapping[in];
}

Medium::SP SemanticParser::createMedium_homogeneous(pbrt::syntactic::Medium::SP in) {
	HomogeneousMedium::SP medium = std::make_shared<HomogeneousMedium>();
	for (auto it : in->param) {
		std::string name = it.first;
		if (name == "g")
			medium->g = in->getParam1f(name, 0);
		else if (name == "scale")
			medium->sigmaScale = in->getParam1f(name, 1);
		else if (name == "Lescale")
			medium->LeScale = in->getParam1f(name, 1);
		else if (name == "sigma_a") {
			if (in->hasParam3f(name)) {
				in->getParam3f(&medium->sigma_a.x, name);
			} else {
				std::size_t N = 0;
				in->getParamPairNf(nullptr, &N, name);
				medium->spec_sigma_a.spd.resize(N);
				in->getParamPairNf(medium->spec_sigma_a.spd.data(), &N, name);
			}
		} else if (name == "sigma_s") {
			if (in->hasParam3f(name)) {
				in->getParam3f(&medium->sigma_s.x, name);
			} else {
				std::size_t N = 0;
				in->getParamPairNf(nullptr, &N, name);
				medium->spec_sigma_s.spd.resize(N);
				in->getParamPairNf(medium->spec_sigma_s.spd.data(), &N, name);
			}			
		} else
			throw std::runtime_error("as-yet-unhandled homogeneous parameter '" + it.first + "'");
	}
	return medium;
}

Medium::SP SemanticParser::createMediumFrom(pbrt::syntactic::Medium::SP in) {
	if (!in) {
		std::cerr << "warning: empty medium!" << std::endl;
		return Medium::SP();
	}

	const std::string type = in->type == "" ? in->getParamString("type") : in->type;

	// ==================================================================
	if (type == "homogeneous") 
		return createMedium_homogeneous(in);

	// ==================================================================
#ifndef NDEBUG
	std::cout << "Warning: un-recognizd medium type '" + type + "'" << std::endl;
#endif
	return std::make_shared<Medium>();
}

} // namespace pbrt