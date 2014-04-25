/// HEADER
#include <utils_vision/utils/extractor_factory.h>

/// COMPONENT
#include <utils_vision/utils/extractor_manager.h>

ExtractorFactory::ExtractorFactory()
    : manager(csapex::ExtractorManager::instance())
{
}

Extractor::Ptr ExtractorFactory::create(const std::string& keypoint, const std::string& descriptor,
                                        const param::ParameterProvider& param)
{
    Extractor::Ptr e(new Extractor(0));

    instance().manager.getInitializer(keypoint, descriptor)->init(e.get(), param);

    return e;
}
