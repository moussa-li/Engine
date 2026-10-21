#pragma once
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <typeinfo>
#include <vector>

namespace EgLab::ME
{

    template <class T>
    struct SoaRegistrar;

    template <class T>
    class SoaContainer
    {
    public:
        using Type = T;

        SoaContainer()
        {
            SoaRegistrar<T>::registerColumns(*this);
        }

        template <class U>
        void addColumn(U T::* member)
        {
            columns_.push_back(std::make_unique<Column<U>>(member));
        }

        void pushBack(const T& obj)
        {
            for (auto& col : columns_)
            {
                col->pushBack(obj);
            }
        }

        template <class U>
        U get(std::size_t index, U T::* member) const
        {
            auto* col = findColumn<U>(member);
            if (!col) throw std::runtime_error("member not found");
            if (index >= size()) throw std::out_of_range("index out of range");
            return col->at(index);
        }

        template <class U>
        void set(std::size_t index, U T::* member, const U& value)
        {
            auto* col = findColumn<U>(member);
            if (!col) throw std::runtime_error("member not found");
            if (index >= size()) throw std::out_of_range("index out of range");
            col->setAt(index, value);
        }

        void deleteAt(std::size_t index)
        {
            if (index >= size()) throw std::out_of_range("index out of range");
            for (auto& col : columns_)
            {
                col->deleteAt(index);
            }
        }

        std::size_t size() const
        {
            return columns_.empty() ? 0 : columns_[0]->size();
        }

    private:
        struct ColumnBase
        {
            virtual ~ColumnBase() = default;
            virtual void pushBack(const T& obj) = 0;
            virtual void deleteAt(std::size_t index) = 0;
            virtual std::size_t size() const = 0;
        };

        template <class U>
        struct Column : ColumnBase
        {
            std::vector<U> data;
            U T::* member;

            Column(U T::* m) : member{m}
            {
            }

            void pushBack(const T& obj) override
            {
                data.push_back(obj.*member);
            }

            void deleteAt(std::size_t index) override
            {
                data.erase(data.begin() + static_cast<std::ptrdiff_t>(index));
            }

            std::size_t size() const override
            {
                return data.size();
            }

            U at(std::size_t index) const
            {
                return data[index];
            }

            void setAt(std::size_t index, const U& value)
            {
                data[index] = value;
            }
        };

        std::vector<std::unique_ptr<ColumnBase>> columns_;

        template <class U>
        const Column<U>* findColumn(U T::* member) const
        {
            for (const auto& col : columns_)
            {
                auto* typedColumn = dynamic_cast<const Column<U>*>(col.get());
                if (typedColumn && typedColumn->member == member)
                {
                    return typedColumn;
                }
            }
            return nullptr;
        }

        template <class U>
        Column<U>* findColumn(U T::* member)
        {
            for (const auto& col : columns_)
            {
                auto* typedColumn = dynamic_cast<Column<U>*>(col.get());
                if (typedColumn && typedColumn->member == member)
                {
                    return typedColumn;
                }
            }
            return nullptr;
        }
    };

#define BEGIN_SOA_REG(Type)                                \
    template <>                                            \
    struct SoaRegistrar<Type>                              \
    {                                                      \
        static void registerColumns(SoaContainer<Type>& c) \
        {
#define SOA_REGISTERED_TYPE(Type) using RegisteredType = Type;
#define END_SOA_REG() \
    }                 \
    }                 \
    ;

#define REG_COL(Type, name) c.addColumn(&RegisteredType::name);

} // namespace EgLab::ME