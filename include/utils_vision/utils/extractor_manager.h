#ifndef EXTRACTOR_MANAGER_H
#define EXTRACTOR_MANAGER_H

/// COMPONENT
#include "extractor.h"
#include <utils_param/parameter_provider.h>

/// SYSTEM
#include <opencv2/opencv.hpp>
#include <boost/function.hpp>
#include <boost/foreach.hpp>
#include <map>

namespace csapex
{
template <typename Any>
class DetectorTraits
{
    typedef char Small;
    class Big
    {
        char dummy[2];
    };

    template <typename Class> static Small testKeypoint(typeof(&Class::keypoint)) ;
    template <typename Class> static Big testKeypoint(...);

    template <typename Class> static Small testDescriptor(typeof(&Class::descriptor)) ;
    template <typename Class> static Big testDescriptor(...);


public:
    enum { HasKeypoint = sizeof(testKeypoint<Any>(0)) == sizeof(Small) };
    enum { HasDescriptor = sizeof(testDescriptor<Any>(0)) == sizeof(Small) };
};
}

#define EXTRACTOR_IMPLEMENTATION \
    std::string getName() const { return name; }\
    static const std::string name;

#define REGISTER_FEATURE_DETECTOR(class_name, impname)\
    const std::string class_name::name(#impname); \
    namespace csapex { \
    class _____##class_name##impname##_registrator { \
    static _____##class_name##impname##_registrator instance; \
    _____##class_name##impname##_registrator () {\
    registerKeypointConstructor<class_name>();\
    registerDescriptorConstructor<class_name>();\
    }\
    template <class T>\
    static void registerKeypointConstructor(typename boost::enable_if_c<csapex::DetectorTraits<T>::HasKeypoint, T>::type* dummy = 0) {\
    ExtractorManager::instance().registerKeypointConstructor(T::name, boost::bind(&T::keypoint, _1, _2, _3)); \
    ExtractorManager::instance().registerKeypointParameters(T::name, boost::bind(&T::usedParameters)); \
    }\
    template <class T>\
    static void registerKeypointConstructor(typename boost::disable_if_c<csapex::DetectorTraits<T>::HasKeypoint, T>::type* dummy = 0) {}\
    template <class T>\
    static void registerDescriptorConstructor(typename boost::enable_if_c<csapex::DetectorTraits<T>::HasDescriptor, T>::type* dummy = 0) {\
    ExtractorManager::instance().registerDescriptorConstructor(T::name, boost::bind(&T::descriptor, _1, _2)); \
    ExtractorManager::instance().registerDescriptorParameters(T::name, boost::bind(&T::usedParameters)); \
    }\
    template <class T>\
    static void registerDescriptorConstructor(typename boost::disable_if_c<csapex::DetectorTraits<T>::HasDescriptor, T>::type* dummy = 0) {}\
    };\
    _____##class_name##impname##_registrator _____##class_name##impname##_registrator::instance;\
    }



namespace csapex
{
/**
 * @brief The ExtractorManager class manages all instances of feature detectors and descriptor extractors.
 *        It functions like a proxy to the underlying singleton classes.
 */
class ExtractorManager : public boost::noncopyable
{
public:
    static ExtractorManager& instance() {
        static ExtractorManager inst;
        return inst;
    }

public:
    /**
     * @brief The ExtractorInitializer
     */
    struct ExtractorInitializer/* : public Constructor*/ {
        typedef boost::function<void(Extractor*, const param::ParameterProvider&, bool)> Call;

        void operator()(Extractor* r, const param::ParameterProvider& param, bool complete = false) const {
            return construct(r, param, complete);
        }

        void construct(Extractor* r, const param::ParameterProvider& param, bool complete = false) const {
            if(!constructor.empty()) {
                constructor(r, param, complete);
            }
        }

        void setConstructor(Call c) {
            constructor = c;
        }


        std::string getType() const {
            return type;
        }

        void setType(const std::string& n) {
            type = n;
        }

        std::string getDescription() const {
            return descr;
        }

        void setDescription(const std::string& n) {
            descr = n;
        }


    private:
        std::string type;
        std::string descr;
        Call constructor;
    };

    struct Params {
        template <typename T>
        T read(const param::ParameterProvider& param, const std::string& name) {
            try {
                param::Parameter::Ptr p = param.getConstParameter(name);
                return p->as<T>();
            } catch(const std::exception& e) {
                BOOST_FOREACH(const param::Parameter::Ptr& p, params) {
                    if(p->name() == name) {
                        return p->as<T>();
                    }
                }
            }

            throw std::out_of_range(std::string("parameter ") + name + " doesn't exist.");
        }

        void add(param::Parameter::Ptr param) {
            params.push_back(param);
        }

        std::vector<param::Parameter::Ptr> params;
    };

    class KeypointInitializer : public ExtractorInitializer {};
    class DescriptorInitializer : public ExtractorInitializer {};

    typedef typename KeypointInitializer::Call KeypointInit;
    typedef typename DescriptorInitializer::Call DescriptorInit;

    typedef boost::function<std::vector<param::Parameter::Ptr>() > ParameterFunction;

private:
    /**
     * @brief ExtractorManager functions like a proxy to the underlying singleton classes.
     */
    ExtractorManager();

public:
    /**
     * @brief ~ExtractorManager
     */
    virtual ~ExtractorManager();

    /**
     * @brief registerKeypointConstructor installs a new keypoint detector instance
     * @param key name of the instance, must be unique
     * @param kc Constructor of the instance, called if <b>key</b> is requested
     */
    void registerKeypointConstructor(const std::string& key, KeypointInit kc);

    /**
     * @brief registerDescriptorConstructor installs a new descriptor extractor instance
     * @param key name of the instance, must be unique
     * @param dc Constructor of the instance, called if <b>key</b> is requested
     */
    void registerDescriptorConstructor(const std::string& key, DescriptorInit dc);


    void registerKeypointParameters(const std::string& key, ParameterFunction f);
    void registerDescriptorParameters(const std::string& des, ParameterFunction f);

    /**
     * @brief getInitializer get a functor to initialize an Extractor
     * @param keypoint name of the keypoint detector
     * @param descriptor name of the descriptor extractor
     * @throws Extractor::IllegalKeypointException iff <b>keypoint</b> is not recognized
     * @throws Extractor::IllegalDescriptorException iff <b>descriptor</b> is not recognized
     * @return
     */
    Extractor::Initializer::Ptr getInitializer(const std::string& keypoint, const std::string& descriptor);

    /**
     * @brief featureDetectors get a container of all detectors
     * @return
     */
    std::map<std::string, KeypointInitializer> featureDetectors() {
        return available_keypoints;
    }

    std::vector<param::Parameter::Ptr> featureDetectorParameters(const std::string& keypoint);
    std::vector<param::Parameter::Ptr> featureDescriptorParameters(const std::string& keypoint);

    /**
     * @brief descriptorExtractors get a container of all extractors
     * @return
     */
    std::map<std::string, DescriptorInitializer> descriptorExtractors() {
        return available_descriptors;
    }

protected:
    struct ExtractorInitializerImp : public Extractor::Initializer {
        ExtractorInitializerImp(KeypointInitializer kc, DescriptorInitializer dc, bool complete)
            : kc_(kc), dc_(dc), complete_(complete)
        {}

        virtual void init(Extractor* e, const param::ParameterProvider& param) {
            if(complete_) {
                kc_(e, param, true);
                // dc_ == kc_
            } else {
                kc_(e, param);
                dc_(e, param);
            }
        }
    private:
        KeypointInitializer kc_;
        DescriptorInitializer dc_;
        bool complete_;
    };

protected:
    std::map<std::string, KeypointInitializer> available_keypoints;
    std::map<std::string, DescriptorInitializer> available_descriptors;

    std::map<std::string, ParameterFunction> param_key;
    std::map<std::string, ParameterFunction> param_des;
};
}

#endif // EXTRACTOR_MANAGER_H
